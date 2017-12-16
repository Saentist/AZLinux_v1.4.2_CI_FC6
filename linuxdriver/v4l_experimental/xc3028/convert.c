#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE(fw) sizeof(fw)/sizeof(fw[0])

struct{
	char *fwname;
	char *fwstart;
	char *fwstart2;
	int length1;
	int length2;
} xc_firmware[]={{"Terratec","\x2a\x03\xe5\xe0\x00\x07\xf4\xd0\x01\xc0\x70\xe0\x00\x07","\x2a\x00\xbc\xe0\x00\x07\xb0\xf1\x05\x01\x67\x82\x02\x82",2480,3890},
	{"Terratec 2006-02-08","\x2a\x03\x9a\xe0\x00\x07\xf4\xd0\x01\xc0\x70\xe0\x00\x07","\x2a\x00\xcf\xe0\x00\x07\xb0\xf1\x05\x01\x78\x82\x02\x82",2632,3852},
	{"Hauppauge","\x2a\x03\xcc\xe0\x00\x07\xf4\xd0\x01\xc0\x70\xe0\x00\x07","\x2a\x00\xbe\xe0\x00\x07\xb9\xf1\x05\x01\x69\x82\x02\x82",2532,3886}};

int main(int argc, char **argv){
	FILE *file;
	char buffer[1024];
	char *fwmem=0;
	int len;
	long fleng=0;
	int i;
	int n=0;
	int x;
	int done=0;
	int d=0;
	int e=0;
	if(argc!=2){
		printf("./convert <filename>\n");
		exit(1);
	}
	file=fopen(argv[1],"r");
	if(!file){
		printf("unable to open file\n");
		exit(1);
	}
	fprintf(stderr,"Firmware extractor 0.1\n");
	while((len=fread(buffer,1,1024,file))){
		fleng+=len;
		fwmem=realloc(fwmem,fleng);
		memcpy(&fwmem[fleng-len],buffer,len);
	}
	fprintf(stderr,"Stored in memory: %ld\n",fleng);
	for(n=0;n<ARRAY_SIZE(xc_firmware);n++){
		for(i=0;i<fleng&&done!=2;i++){
			if(xc_firmware[n].fwstart[d]==fwmem[i]&&done!=1){
				if(d==13){
					fprintf(stderr,"==== %s ====\n",xc_firmware[n].fwname);
					printf("%d\n",xc_firmware[n].length1);
					fprintf(stderr,"FW part 1 found!\n");
					fprintf(stderr,"Length: %d bytes\n",xc_firmware[n].length1);
					for(x=1;x<=xc_firmware[n].length1;x++){
						fprintf(stderr,"%02x ",((unsigned char*)fwmem)[i-13+x]);
						printf("%c",((unsigned char*)fwmem)[i-13+x]);
						if(x%63==0){
							fprintf(stderr,"\n");
						}
					}
					fprintf(stderr,"\n");
					done++;
				}
				d++;
			} else {
				d=0;
			}
			if(xc_firmware[n].fwstart2[e]==fwmem[i]){
				if(e==13){
					fprintf(stderr,"-------------------------------------------\n");
					fprintf(stderr,"FW part 2 found!\n");
					fprintf(stderr,"Length: %d bytes\n",xc_firmware[n].length2);
					for(x=1;x<=xc_firmware[n].length2;x++){
						printf("%c",((unsigned char*)fwmem)[i-13+x]);
						fprintf(stderr,"%02x ",((unsigned char*)fwmem)[i-13+x]);
						if(x%63==0){
							fprintf(stderr,"\n");
						}
					}
					done++;
				}
				e++;
			} else {
				e=0;
			}
		}
	}
	free(fwmem);
	fclose(file);
	exit(0);
}
