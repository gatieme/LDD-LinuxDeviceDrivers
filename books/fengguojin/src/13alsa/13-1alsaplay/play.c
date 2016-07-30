#include<stdio.h>
#include<stdlib.h>
#include<alsa/asoundlib.h>
int main(int argc,char *argv[])
{
    int i=0;
    int err;
    char buf[128];
    snd_pcm_t *playback_handle;
    int rate=22050;
    int channels=2;
    snd_pcm_hw_params_t *hw_params;
    if((err=snd_pcm_open(&playback_handle,argv[1],SND_PCM_STREAM_PLAYBACK,0))<0){
        fprintf(stderr,"cant open audio device %s (%s)\n",argv[1],snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params_malloc(&hw_params))<0){
        fprintf(stderr,"cant open allocate parameter structure  (%s)\n",snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params_any(playback_handle,hw_params))<0){
        fprintf(stderr,"cant open initialize hw parameter structure  (%s)\n",snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params_set_access(playback_handle,hw_params,SND_PCM_ACCESS_RW_INTERLEAVED))<0){
        fprintf(stderr,"cant set access type  (%s)\n",snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params_set_format(playback_handle,hw_params,SND_PCM_FORMAT_S16_LE))<0){
        fprintf(stderr,"cant open set format (%s)\n",snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params_set_rate_near(playback_handle,hw_params,&rate,0))<0){
        fprintf(stderr,"cant set rate (%s)\n",snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params_set_channels(playback_handle,hw_params,channels))<0){
        fprintf(stderr,"cant set channels  (%s)\n",snd_strerror(err));
        exit(1);
    }
    if((err=snd_pcm_hw_params(playback_handle,hw_params))<0){
        fprintf(stderr,"cant open set parameter (%s)\n",snd_strerror(err));
        exit(1);
    }
    snd_pcm_hw_params_free(hw_params);
    if((err=snd_pcm_prepare(playback_handle))<0){
        fprintf(stderr,"cant prepare audio interface for use  (%s)\n",snd_strerror(err));
        exit(1);
    }
    i=0;
    while(i<256)
	{
		memset(buf,i,128);
		err=snd_pcm_writei(playback_handle,buf,32);
		fprintf(stderr,"write to audio interface %d\n",err);
		if(err<0)
		{
			snd_pcm_prepare(playback_handle);
		}
		i++;
	}
	snd_pcm_close(playback_handle);
    exit(0);
}
