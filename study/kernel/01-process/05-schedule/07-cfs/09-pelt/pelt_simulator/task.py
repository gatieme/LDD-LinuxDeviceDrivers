class Task(object):

    pelt_sample_us = 1024
    pelt_max = 1024

    def __init__(self, period_ps, run_ps, start_ps=0):
        """
        A task which activations are specified in terms
        of PELT samples.
        A PELT sample is 1.024 ms long, this is the time
        interval used by the Linux scheduler to update a
        PELT signal.

        :param start_ps: task start time
        :type start_ps: int

        :param period_ps: the period of each activation
        :type period_ps: int

        :param run_ps: the duration of each activation
        :type run_ps: int
        """
        self.start_us  = Task.pelt_sample_us * start_ps
        self.period_us = Task.pelt_sample_us * period_ps
        self.run_us    = Task.pelt_sample_us * run_ps

        self.idle_us   = self.period_us - self.run_us
        self.duty_cycle_pct = int(100. * run_ps / period_ps)
        self.util_avg =  int(float(Task.pelt_max) * self.duty_cycle_pct / 100)

    def running(self, time_ms):
        #
        #    +----+    +----+
        #    |    |    |    |
        #         +----+    +----
        #   -+----+----+----+------->
        #    1    2    3    4      PELT Samples
        #    R    S    R    S      Task Status
        #   ^    ^    ^    ^       Task running sampling time
        #
        #
        # NOTE: Since we sample exactly at PELT samples intervals, the exact time
        # when a task starts to run we consider it as if if was running for the entire
        # previous PELT interval. To fix this, by exploting the knowledge that a task
        # is always configured using an integer number of run/speep PELT samples, we can
        # look at the 1us before time wrt the specified sample time.
        # The 1us before time is always represeting the exact status of the task
        # in that period. This is why we remove 1 in the following computation:
        period_delta_us = (float(time_ms) * 1e3 - 1) % self.period_us
        if period_delta_us <= self.run_us:
            return True
        return False

    def __str__(self):
        return "Task(start: {:.3f} [ms], period: {:.3f} [ms], run: {:.3f} [ms], "\
               "duty_cycle: {:.3f} [%], util_avg: {:d})"\
                .format(self.start_us/1e3, self.period_us/1e3,
                        self.run_us/1e3, self.duty_cycle_pct,
                        self.util_avg)

class PeriodicTask(Task):
    """
    A periodic task which is specified by period and duty-cycle.

    :see also: Task
    """
    def __init__(self, period_ps, duty_cycle_pct, start_ps=0):
        run_ps = period_ps * float(duty_cycle_pct) / 100
        super(PeriodicTask, self).__init__(period_ps, run_ps, start_ps)

class AndroidTask(Task):
    """
    An "android" task which is specified by run-time over a (default) period
    of 16ms.

    :see also: Task
    """
    def __init__(self, run_ps, start_ps=0, period_ps=16):
        super(AndroidTask, self).__init__(period_ps, run_ps, start_ps)


if __name__ == "__main__":
    t1 = Task(start_ps=1, period_ps=16, run_ps=4)
    print(t1)

    t2 = PeriodicTask(start_ps=1, period_ps=16, duty_cycle_pct=1)
    print(t2)

    t3 = AndroidTask(run_ps=4)
    print(t3)

    run = []
    for t_ps in range(1,16):
        run.append(t3.running(t_ps * 1.024))
    print(run)
