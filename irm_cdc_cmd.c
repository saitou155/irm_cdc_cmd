#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libserialport.h>
#include <json-c/json.h>

#define VENDOR_ID		0x04d8
#define PRODUCT_ID	0x000a

struct sp_port * port = NULL;
int varbose = 0;
void dump(char cType, const unsigned char* data,int size);
void usage();
int irm_flush();

int irm_open()
{
	int idx = 0;
	struct sp_port **ports;
	struct sp_port *port0 = NULL;
	enum sp_return ret;
	int i;
	int cnt = -1;

	ret = sp_list_ports(&ports);
	if(ret != SP_OK)
	{
		fprintf(stderr,"ERROR:%d = sp_list_ports() %d %s\n", ret, sp_last_error_code(), sp_last_error_message());
		return -1;
	}

	for (i = 0; ports[i]; i++)
	{
		int vid,pid;
		ret = sp_get_port_usb_vid_pid (ports[i], &vid, &pid);
		if(!ret && vid == VENDOR_ID && pid == PRODUCT_ID)
		{
			cnt++;
			if(cnt == idx)
			{
				sp_get_port_by_name(sp_get_port_name(ports[i]), &port0);
				break;
			}
		}
	}
	sp_free_port_list(ports);
	if(!port0)
	{
		fprintf(stderr,"ERROR:Can't find serial port. vid:%04X pid:%04X\n", VENDOR_ID, PRODUCT_ID);
		return -1;
	}

	ret = sp_open(port0,SP_MODE_READ_WRITE);
	if(ret != SP_OK)
	{
		fprintf(stderr,"ERROR:%d = sp_open() %d %s\n", ret, sp_last_error_code(), sp_last_error_message());
		sp_free_port(port0);
		return -1;
	}

	struct sp_port_config * config = NULL;
	ret = sp_new_config(&config);
	if(ret != SP_OK)
	{
		fprintf(stderr,"ERROR:%d = sp_new_config() %d %s\n", ret, sp_last_error_code(), sp_last_error_message());
	}
	sp_get_config(port0,config);
	sp_set_config_baudrate(config,115200);
	sp_set_config_bits(config,8);
	sp_set_config_parity(config,SP_PARITY_NONE);
	//sp_set_config_rts(config,SP_RTS_ON);
	//sp_set_config_cts(config,SP_CTS_IGNORE);
	//sp_set_config_dtr(config,SP_DTR_ON);
	sp_set_config_xon_xoff(config,SP_XONXOFF_DISABLED);
	ret = sp_set_config(port0,config);
	if(ret != SP_OK)
	{
		fprintf(stderr,"ERROR:%d = sp_set_config() %d %s\n", ret, sp_last_error_code(), sp_last_error_message());
	}
	sp_free_config(config);

	port = port0;

	irm_flush();

	return 0;
}

int irm_close()
{
	enum sp_return ret;

	ret = sp_close(port);
	if(ret != SP_OK)
	{
		return -1;
	}
	sp_free_port(port);
	port = NULL;

	return 0;
}


int irm_puts(unsigned char * data, int size)
{
	enum sp_return ret;
	ret = sp_blocking_write(port, data, size,1000);
	if(ret != size)
	{
	  return -1;
	}
	if(varbose)
	{
		dump('S',data,size);
	}
	return ret;
}

int irm_gets(unsigned char * data, int size)
{
	int mS = 200;
	int res = 0;
	int i,ret,count;
	unsigned char * poi;
	size_t remain;

	memset(data,0,size);

	count = 0;
	poi = data + count;
	remain = size - count;
	for(i = 0;;i++)
	{
		if(remain <= 0)
		{
			break;
		}
		if(strstr((const char*)data,"\r\n") != NULL)
		{
			break;
		}
		ret = sp_blocking_read_next(port,poi, remain, mS);
		if(ret > 0)
		{
			count += ret;
			poi = data + count;
			remain = size - count;
			res = count;
		}
		else if(ret == 0)
		{
			if(i != 0)
			{
				break;
			}
		}
		else //if(ret == 0)
		{
			res = ret;
			break;
		}
	}
	if(res >= 0)
	{
		if(varbose)
		{
			dump('R',data,res);
		}
	}
	return res;
}

int irm_flush()
{
	enum sp_return ret;
	ret = sp_flush(port,SP_BUF_BOTH);
	if(ret != SP_OK)
	{
		return -1;
	}
	return 0;
}

int irm_cmd(const char* cmd1)
{
	int result = -1;
	int ret;

	ret = irm_puts((unsigned char*)cmd1, strlen(cmd1));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	result = 0;
EXIT_PATH:
	return result;
}

int irm_cmd_res2(const char* cmd1,const char* res1,char* data,int size)
{
	int result = -1;
	int ret;
	char buf[256];

	ret = irm_puts((unsigned char*)cmd1, strlen(cmd1));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	usleep(10 * 1000);

	ret = irm_gets((unsigned char *)buf,sizeof(buf));
	if(ret < 0)
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}
	if(strcmp(buf,res1))
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}

	usleep(3 * 1000 * 1000);

	ret = irm_gets((unsigned char *)data,size);
	if(ret < 0)
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}

	result = ret;
EXIT_PATH:
	return result;
}

int irm_cmd_res(const char* cmd1,char* data,int size)
{
	int result = -1;
	int ret;

	ret = irm_puts((unsigned char *)cmd1, strlen(cmd1));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	ret = irm_gets((unsigned char *)data,size);
	if(ret < 0)
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}
	result = ret;
EXIT_PATH:
	return result;
}

int irm_receive()
{
	int result = -1;
	int ret;
	int len,recNo,pScale,mem;
	char cmd[32];
	char buf[32];
	unsigned char range[640];

	fprintf(stderr,"Capturing IR...\n");

	ret = irm_open();
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	irm_flush();

	ret = irm_cmd_res2("c\r\n","...",buf,sizeof(buf));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	len = -1;
	ret = sscanf(buf," %d",&len);
	if(ret < 1 || len < 1)
	{
		if(!varbose)
		{
			fprintf(stderr," Time Out !\n");
		}
		goto EXIT_PATH;
	}

	ret = irm_cmd_res("I,1\r\n",buf,sizeof(buf));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	recNo = -1;
	ret = sscanf(buf," %x",&recNo);
	if(ret < 1 || recNo < 1)
	{
		goto EXIT_PATH;
	}

	ret = irm_cmd_res("I,6\r\n",buf,sizeof(buf));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	pScale = -1;
	ret = sscanf(buf," %d",&pScale);
	if(ret < 1 || pScale < 1)
	{
		goto EXIT_PATH;
	}

	for(int i = 0;i < recNo;i++)
	{
		int nPage = i / 64;
		int nOff  = i % 64;

		if(!nOff)
		{
			sprintf(cmd,"B,%d\r\n",nPage);
			ret = irm_cmd(cmd);
			if(ret < 0)
			{
				goto EXIT_PATH;
			}
		}
		sprintf(cmd,"D,%d\r\n",nOff);
		ret = irm_cmd_res(cmd,buf,3);
		if(ret < 0)
		{
			goto EXIT_PATH;
		}
		mem = -1;
		ret = sscanf(buf," %x",&mem);
		if(ret < 1 || mem < 1)
		{
			goto EXIT_PATH;
		}
		range[i] = (unsigned char)(mem & 0xff);
	}

	json_object *pJSON = json_object_new_object();
	json_object_object_add(pJSON, "format", json_object_new_string("raw"));
	json_object_object_add(pJSON, "freq", json_object_new_int(38));
	json_object *pARRAY = json_object_new_array();
	for(int i = 0;i < recNo;i++)
	{
		json_object_array_add(pARRAY,json_object_new_int(range[i]));
	}
	json_object_object_add(pJSON, "data", pARRAY);
	json_object_object_add(pJSON, "postscale", json_object_new_int(pScale));
	printf("%s\n",json_object_to_json_string(pJSON));
	json_object_put(pJSON);

	result = 0;
EXIT_PATH:
	irm_close();
	return result;
}

int irm_transfer(const char* json)
{
	int result = -1;

	json_object *pJSON;
	json_object * pDATA;
	json_object * pSCALE;
	json_object * pITEM;
	char cmd[32];
	char buf[32];

	int ret;
	int recNo,pScale;
	unsigned char range[640];

	pJSON = json_tokener_parse(json);
	if(pJSON == NULL)
	{
		if(!varbose)
		{
			fprintf(stderr,"ERROR:Can't parse json.\n");
		}
		else
		{
			fprintf(stderr,"ERROR:Can't parse json.\"%s\"\n",json);
		}
		goto EXIT_PATH;
	}

	if(!json_object_object_get_ex(pJSON, "postscale",&pSCALE))
	{
		fprintf(stderr,"ERROR:not found postscale in json.\n");
		goto EXIT_PATH;
	}
	if(json_object_get_type(pSCALE) != json_type_int)
	{
		fprintf(stderr,"ERROR:postscale was not integer.\n");
		goto EXIT_PATH;
	}
	pScale = json_object_get_int(pSCALE);

	if(!json_object_object_get_ex(pJSON, "data",&pDATA))
	{
		fprintf(stderr,"ERROR:not found data in json.\n");
		goto EXIT_PATH;
	}
	if(json_object_get_type(pDATA) != json_type_array)
	{
		fprintf(stderr,"ERROR:data was not array.\n");
		goto EXIT_PATH;
	}
	recNo = json_object_array_length(pDATA);
	for(int i = 0;i < recNo;i++)
	{
		pITEM = json_object_array_get_idx(pDATA, i);
		if (json_object_get_type(pITEM) != json_type_int)
		{
			fprintf(stderr,"ERROR:data[%d] was not integer.\n",i);
			goto EXIT_PATH;
		}
		range[i] = (char)(json_object_get_int(pITEM) & 0xff);
	}
	json_object_put(pJSON);

	fprintf(stderr,"Transfer IR...\n");

	ret = irm_open();
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	irm_flush();

	sprintf(cmd,"N,%d\r\n",recNo);
	ret = irm_cmd_res(cmd,buf,sizeof(buf));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(strcmp(buf,"OK\r\n"))
	{
		goto EXIT_PATH;
	}

	sprintf(cmd,"K,%d\r\n",pScale);
	ret = irm_cmd_res(cmd,buf,sizeof(buf));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(strcmp(buf,"OK\r\n"))
	{
		goto EXIT_PATH;
	}

	for(int i = 0;i < recNo;i++)
	{
		int nPage = i / 64;
		int nOff  = i % 64;

		if(!nOff)
		{
			sprintf(cmd,"B,%d\r\n",nPage);
			ret = irm_cmd(cmd);
			if(ret < 0)
			{
				goto EXIT_PATH;
			}
		}
		sprintf(cmd,"W,%d,%d\r\n",nOff,(int)range[i]);
		ret = irm_cmd(cmd);
		if(ret < 0)
		{
			goto EXIT_PATH;
		}
	}

	ret = irm_cmd_res("P\r\n",buf,sizeof(buf));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(strcmp(buf,"..."))
	{
		goto EXIT_PATH;
	}

EXIT_PATH:
	irm_close();
	return result;
}

int main(int argc,char * argv[])
{
	int ret = -1;
	char cmd = '\0';
	char* json = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "vrt:")) != -1)
	{
        switch (opt)
		{
            case 'r':
				cmd = opt;
                break;
            case 'v':
				varbose = 1;
                break;
            case 't':
				cmd = opt;
				json = optarg;
                break;
        }
    }

	if(cmd == '\0')
	{
		usage();
		return 0;
	}

	switch(cmd)
	{
		case 'r':
			ret = irm_receive();
			break;
		case 't':
			ret = irm_transfer(json);
			break;
	}

	return ret;
}

void usage()
{
  fprintf(stderr, "usage: irm_ir_cmd <option>\n");
  fprintf(stderr, "  -r       \tReceive Infrared code.\n");
  fprintf(stderr, "  -t 'json'\tTransfer Infrared code.\n");
  fprintf(stderr, "  -t \"$(cat XXX.json)\"\n");
  fprintf(stderr, "  -t \"`cat XXX.json`\"\n");
  fprintf(stderr, "  -v       \tVerbose mode.\n");
}

void dump(char cType, const unsigned char* data,int size)
{
	char c;
	int i,j;
	for(i = 0;i < size;i+=16)
	{
		fprintf(stderr,"%c %04X ",cType,i);
		for(j = 0;j < 16;j++)
		{
			if((i + j) >= size)
			{
				break;
			}
			c = data[i + j];
			fprintf(stderr,"%02X ",(int)c & 0xFF);
			if(j == 7)
			{
			 fprintf(stderr,"- ");
			}
		}
		for(/*j = 0*/;j < 16;j++)
		{
			fprintf(stderr,"   ");
			if(j == 7)
			{
				fprintf(stderr,"- ");
			}
		}
		for(j = 0;j < 16;j++)
		{
			if((i + j) >= size)
			{
				break;
			}
			c = data[i + j];
			fprintf(stderr,"%c",isprint(c) ? c : '?');
			if(j == 7)
			{
				fprintf(stderr," ");
			}
		}
		for(/*j = 0*/;j < 16;j++)
		{
			fprintf(stderr," ");
			if(j == 7)
			{
				fprintf(stderr," ");
			}
		}
		fprintf(stderr,"\n");
	}
}
