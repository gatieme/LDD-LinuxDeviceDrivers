from collections import namedtuple
import pandas as pd
import matplotlib.pyplot as plt

from devlib.utils.misc import memoized
from collections import namedtuple

PELTStats = namedtuple('PELTStats', [
    'util_avg', 'util_init', 'half_life',
    'tmin', 'tmax', 'min', 'max', 'avg', 'std',
    'err', 'err_pct'
])

class PELT(object):
    """
    A simple PELT simulator which allows to compute and plot the PELT signal
    for a specifed (periodic) task. The simulator can be configured using the
    main settings exposed by PELT: the half-life of a task and its intial
    value. The capping of the decay is an experimental features which is also
    supported by the current model.

    The model is pretty simple and works by generating a new value at each
    PELT sample interval (_sample_us) specified in [us] (1024 by default).

    The model allows to compute statistics on the generated signal, over a
    specified time frame. It provides also an embedded support to plot the
    generated signal along with a set of expected thresholds and values
    (e.g. half-life, min/max stability values)
    """

    _sample_us = 1024
    _signal_max = 1024

    def __init__(self, task, init_util=0, half_life_ms=32,
                 decay_cap_ms=None, verbose=False):
        """
        Initialize the PELT simulator for a specified task.

        :param task:         the task we want to simulate the PELT signal for
        :type  task: Task

        :param init_util:    the initial utilization for the task
        :type  init_util: int

        :param half_life_ms: the [ms] interval required by a signal to
                             increased/decreased by half of its range
        :type  half_life_ms: int

        :param decay_cap_ms: the [ms] interval after which we do not decay
                             further a signal
        :type  decay_cap_ms: int

        """
        self.task = task
        self.init_util = init_util
        self.half_life_ms = half_life_ms
        self.decay_cap_ms = decay_cap_ms

        self.geom_y = pow(0.5, 1./half_life_ms)
        self.geom_u = float(PELT._signal_max) * (1. - self.geom_y)

        self.df = None

        self._verbose = verbose
        if not verbose:
            return

        print("PELT Configured with   :")
        print(("  initial utilization  : ", self.init_util))
        print(("  half life (HL)  [ms] : ", self.half_life_ms))
        print(("  decay capping @ [ms] : ", self.decay_cap_ms))
        print(("  y =    0.5^(1/HL)    : {:6.3f}".format(self.geom_y)))
        print(("  u = {:5d}*(1 - y)    : {:6.3f}".format(PELT._signal_max, self.geom_u)))
        print("Task configured with   :")
        print(("  run_time [us]        : ", self.task.run_us))
        print(("  period [us]          : ", self.task.period_us))
        print(("  duty_cycle [%]       : ", self.task.duty_cycle_pct))
        (min_val, max_val) = self.stable_range()
        print(("  stable range         : ({:.1f}, {:.1f})".format(min_val, max_val)))

    def geom_sum(self, u_1, active_us=0):
        """
        Geometric add the specified value to the u_1 series.

        NOTE: the current implementation assume that the task was active
              (active_us != 0) or sleeping (active_us == 0) a full PELT
              sampling interval (by default 1024[us])
        """
        # Decay previous samples
        u_1 *= self.geom_y
        # Add current sample (if task was active)
        if active_us:
            u_1 += self.geom_u
        return u_1

    def signal(self, start_s=0, end_s=10):
        """
        Compute the PELT signal for the specified interval.

        :param start_s: the start time in [s]
        :type  start_s: float

        :param end_s: the end time in [s]
        :type  end_s: float
        """
        # Computed PELT samples
        samples = []
        # Decay capping support
        running_prev = True
        capping = None

        # Intervals (in PELT samples) for the signal to compute
        start_us = int(1e6 * PELT._sample_us/1e3 * start_s)
        end_us   = int(1e6 * PELT._sample_us/1e3 * end_s)
        if self._verbose:
            print(("Signal Range : ({}:{}) [us]".format(start_us, end_us)))

        # Add initial value to the generated signal
        util = self.init_util
        t_us = start_us
        samples.append((float(t_us)/1e6, 0, False, util))

        # Compute following PELT samples
        t_us = start_us + PELT._sample_us
        while t_us <= end_us:

            # Check if the task was running in the current PELT sample
            running = self.task.running(float(t_us/1e3))

            # Keep track of sleep start and decay capping time
            if self.decay_cap_ms and running_prev and not running:
                capping = t_us + (self.decay_cap_ms * 1e3)
                #print "Set capping @{}".format(t_capping)

            # Assume the task was running for all the current PELT sample
            active_us = PELT._sample_us if running else 0

            # Always update utilization:
            # - when the task is running
            # - when is not running and we are not capping the decay
            if running or not self.decay_cap_ms:
                util = self.geom_sum(util, active_us)

            # Update decay only up to the capping point
            elif capping and t_us < capping:
                util = self.geom_sum(util, active_us)

            # Keep tracking the decay capped value
            #else:
            #    print "{}: capped @{} (since {})".format(t_us, util, capping)

            # Append PELT samples
            samples.append((float(t_us)/1e6, t_us/PELT._sample_us, running, util))

            # Prepare for next sample computation
            running_prev = running
            t_us += PELT._sample_us

        # Create DataFrame from computed samples
        self.df = pd.DataFrame(samples, columns=['Time', 'PELT_Interval', 'Running', 'util_avg'])
        self.df.set_index('Time', inplace=True)

        return self.df

    def stats(self, start_s=0, end_s=None):
        """
        Compute the stats over a pre-computed PELT signal, considering only
        the specified portion of the signal.

        :param start_s: the start of signal portion to build the stats for
        :type  start_s: float

        :param end_s: the end of signal portion to build the stats for
        :type  end_s: float
        """

        # Intervals (in PELT samples) for the statistics to compute
        start_s *= float(self._sample_us)/1e3
        start_s = max(start_s, self.df.index.min())
        if end_s:
            end_s *= float(self._sample_us)/1e3
            end_s = min(end_s, self.df.index.max())
        else:
            end_s = self.df.index.max()

        df = self.df[start_s:end_s]
        if self._verbose:
            print(("Stats Range  : ({:.6f}:{:.6f}) [s]".format(start_s, end_s)))

        # Compute stats
        _util_avg = self.task.util_avg
        _util_init = self.init_util
        _half_life = self.half_life_ms
        _tmin = start_s
        _tmax = end_s
        _min = df.util_avg.min()
        _max = df.util_avg.max()
        _avg = df.util_avg.mean()
        _std = df.util_avg.std()
        _err = _avg - self.task.util_avg
        _err_pct = None
        if self.task.util_avg:
            _err_pct = 100 * _err / self.task.util_avg

        return PELTStats(
            _util_avg, _util_init, _half_life,
            _tmin, _tmax, _min, _max,
            _avg, _std, _err, _err_pct)

    def plot(self, start_s=0, end_s=10, stats_start_s=0, stats_end_s=None):
        """
        Plot a PELT signal for the specified interval and report stats for a
        possibly different portion of the signal.

        NOTE: the portion of signal used for statistics is highlighted in
              yellow in the generated plot.

        :param start_s: the start time in [s]
        :type  start_s: float

        :param end_s: the end time in [s]
        :type  end_s: float

        :param stats_start_s: the start of signal portion to build the stats for
        :type  stats_start_s: float

        :param stats_end_s: the end of signal portion to build the stats for
        :type  stats_end_s: float
        """

        # Generate PELT signal for the task
        self.signal(start_s, end_s)
        pelt_stats = self.stats(stats_start_s, stats_end_s)

        # Plot PELT signal and half life marker
        fig, axes = plt.subplots(1, 1, figsize=(16, 8))

        # Plot simulated PELT signal
        self.df.util_avg.plot(ax=axes)

        # Plot HALF-LIFE vertical marker
        hl_s  = float(self.half_life_ms) * PELT._sample_us/1e3
        hl_s /= 1e3
        axes.vlines(hl_s, 0, 1024, colors=['r'], linestyle='--')

        # Plot expected MIN/MAX stable variation markers
        (util_min, util_max) = self.stable_range()
        axes.hlines(util_min, 0, 1024, colors=['r'], linestyle='--')
        axes.hlines(util_max, 0, 1024, colors=['r'], linestyle='--')

        # Plot AVERAGE horizontal marker
        axes.hlines(pelt_stats.avg, 0, 1024, colors=['c'], linestyle='--')
        # Plot horizontal +-1% EXPECTED UTILIZATION bands
        axes.axhspan(self.task.util_avg - 10,
                     self.task.util_avg + 10,
                     facecolor='g', alpha=0.1)

        # Plot STATS vertical band
        axes.axvspan(pelt_stats.tmin, pelt_stats.tmax,
                     facecolor='y', alpha=0.1)

        # Plot title with stats
        err_str = "{:.1f} [%]".format(pelt_stats.err_pct) if pelt_stats.err_pct else "NaN"
        axes.set_title('Task uitl: {}, run {} [ms] every {} [ms], avg: {:.1f}, '\
                       'err: {}, min: {:.1f}, max: {:.1f}'.format(
                int(self.task.util_avg),
                int(float(self.task.run_us)/1e3),
                int(float(self.task.period_us)/1e3),
                pelt_stats.avg, err_str,
                pelt_stats.min, pelt_stats.max
            ))

        #return pelt

    def __str__(self):
        """
        Return a textual description of the PELT signal for the specified task
        """
        print("Task configuration:")
        print(("   ", self.task))

        print("PELT engine configuration:")
        print(("  half-life: {} ms".format(self.half_life_ms)))
        print(("  init_util: {}".format(self.init_util)))

        # Report stats
        print("Utilization signal:")
        print(("  [min,max]: {:5.1f}, {:5.1f}".format(self.min, self.max)))
        print(("  [avg,std]: {:5.1f} (+- {:5.1f})".format(self.avg, self.std)))
        print(("   expected: {:5.1f}".format(self.task.util)))
        print(("      error: {:5.1f} ({:.1f} %)".format(self.err, self.err_pct)))
        return ""

    def stable_range(self):
        """
        Compute the stability ranges for the specified (periodic) task

        Here we use:
        - Decay factor:       y = 0.5^(1/half_life)
        - Max stable value:       (1-y^r) / (1-y^p)
        - Min stable value: y^i * (1-y^r) / (1-y^p)
        """

        max_util  = (1. - pow(self.geom_y, float(self.task.run_us)/PELT._signal_max))
        max_util /= (1. - pow(self.geom_y, float(self.task.period_us)/PELT._signal_max))

        min_util  = (max_util * pow(self.geom_y, float(self.task.idle_us)/PELT._signal_max))

        return (PELT._signal_max * min_util, PELT._signal_max * max_util)

if __name__ == "__main__":
    print("PELT")
