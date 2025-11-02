/*
 *  Copyright (C) 2002-2021  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "dosbox.h"
#include "regs.h"
#include "control.h"
#include "shell.h"
#include "callback.h"
#include "support.h"

extern Bitu PIC_Ticks;

Bitu call_shellstop;
/* Larger scope so shell_del autoexec can use it to
 * remove things from the environment */
DOS_Shell * first_shell = 0;

static Bitu shellstop_handler(void) {
	return CBRET_STOP;
}

static void SHELL_ProgramStart(Program * * make) {
	*make = new DOS_Shell;
}
//Repeat it with the correct type, could do it in the function below, but this way it should be 
//clear that if the above function is changed, this function might need a change as well.
static void SHELL_ProgramStart_First_shell(DOS_Shell * * make) {
	*make = new DOS_Shell;
}

#define AUTOEXEC_SIZE 4096
static char autoexec_data[AUTOEXEC_SIZE] = { 0 };
static std::list<std::string> autoexec_strings;
typedef std::list<std::string>::iterator auto_it;

void VFILE_Remove(const char *name);

void AutoexecObject::Install(const std::string &in) {
	if(GCC_UNLIKELY(installed)) E_Exit("autoexec: already created %s",buf.c_str());
	installed = true;
	buf = in;
	autoexec_strings.push_back(buf);
	this->CreateAutoexec();

	//autoexec.bat is normally created AUTOEXEC_Init.
	//But if we are already running (first_shell)
	//we have to update the envirionment to display changes

	if(first_shell)	{
		//create a copy as the string will be modified
		std::string::size_type n = buf.size();
		char* buf2 = new char[n + 1];
		safe_strncpy(buf2, buf.c_str(), n + 1);
		if((strncasecmp(buf2,"set ",4) == 0) && (strlen(buf2) > 4)){
			char* after_set = buf2 + 4;//move to variable that is being set
			char* test = strpbrk(after_set,"=");
			if(!test) {first_shell->SetEnv(after_set,"");return;}
			*test++ = 0;
			//If the shell is running/exists update the environment
			first_shell->SetEnv(after_set,test);
		}
		delete [] buf2;
	}
}

void AutoexecObject::InstallBefore(const std::string &in) {
	if(GCC_UNLIKELY(installed)) E_Exit("autoexec: already created %s",buf.c_str());
	installed = true;
	buf = in;
	autoexec_strings.push_front(buf);
	this->CreateAutoexec();
}

void AutoexecObject::CreateAutoexec(void) {
	/* Remove old autoexec.bat if the shell exists */
	if(first_shell)	VFILE_Remove("AUTOEXEC.BAT");

	//Create a new autoexec.bat
	autoexec_data[0] = 0;
	size_t auto_len;
	for(auto_it it = autoexec_strings.begin(); it != autoexec_strings.end(); it++) {

		std::string linecopy = (*it);
		std::string::size_type offset = 0;
		//Lets have \r\n as line ends in autoexec.bat.
		while(offset < linecopy.length()) {
			std::string::size_type  n = linecopy.find("\n",offset);
			if ( n == std::string::npos ) break;
			std::string::size_type rn = linecopy.find("\r\n",offset);
			if ( rn != std::string::npos && rn + 1 == n) {offset = n + 1; continue;}
			// \n found without matching \r
			linecopy.replace(n,1,"\r\n");
			offset = n + 2;
		}

		auto_len = strlen(autoexec_data);
		if ((auto_len+linecopy.length() + 3) > AUTOEXEC_SIZE) {
			E_Exit("SYSTEM:Autoexec.bat file overflow");
		}
		sprintf((autoexec_data + auto_len),"%s\r\n",linecopy.c_str());
	}
	if (first_shell) VFILE_Register("AUTOEXEC.BAT",(Bit8u *)autoexec_data,(Bit32u)strlen(autoexec_data));
}

AutoexecObject::~AutoexecObject(){
	if(!installed) return;

	// Remove the line from the autoexecbuffer and update environment
	for(auto_it it = autoexec_strings.begin(); it != autoexec_strings.end(); ) {
		if ((*it) == buf) {
			std::string::size_type n = buf.size();
			char* buf2 = new char[n + 1];
			safe_strncpy(buf2, buf.c_str(), n + 1);
			bool stringset = false;
			// If it's a environment variable remove it from there as well
			if ((strncasecmp(buf2,"set ",4) == 0) && (strlen(buf2) > 4)){
				char* after_set = buf2 + 4;//move to variable that is being set
				char* test = strpbrk(after_set,"=");
				if (!test) {
					delete [] buf2;
					continue;
				}
				*test = 0;
				stringset = true;
				//If the shell is running/exists update the environment
				if (first_shell) first_shell->SetEnv(after_set,"");
			}
			delete [] buf2;
			if (stringset && first_shell && first_shell->bf && first_shell->bf->filename.find("AUTOEXEC.BAT") != std::string::npos) {
				//Replace entry with spaces if it is a set and from autoexec.bat, as else the location counter will be off.
				*it = buf.assign(buf.size(),' ');
				it++;
			} else {
				it = autoexec_strings.erase(it);
			}
		} else it++;
	}
	this->CreateAutoexec();
}

DOS_Shell::DOS_Shell():Program(){
	input_handle=STDIN;
	echo=true;
	exit=false;
	bf=0;
	call=false;
	completion_start = NULL;
}

//DBP: memory cleanup
DOS_Shell::~DOS_Shell(){
	while (bf) delete bf;
}

Bitu DOS_Shell::GetRedirection(char *s, char **ifn, char **ofn,bool * append) {

	char * lr=s;
	char * lw=s;
	char ch;
	Bitu num=0;
	bool quote = false;
	char* t;

	while ( (ch=*lr++) ) {
		if(quote && ch != '"') { /* don't parse redirection within quotes. Not perfect yet. Escaped quotes will mess the count up */
			*lw++ = ch;
			continue;
		}

		switch (ch) {
		case '"':
			quote = !quote;
			break;
		case '>':
			*append=((*lr)=='>');
			if (*append) lr++;
			lr=ltrim(lr);
			if (*ofn) free(*ofn);
			*ofn=lr;
			while (*lr && *lr!=' ' && *lr!='<' && *lr!='|') lr++;
			//if it ends on a : => remove it.
			if((*ofn != lr) && (lr[-1] == ':')) lr[-1] = 0;
//			if(*lr && *(lr+1))
//				*lr++=0;
//			else
//				*lr=0;
			t = (char*)malloc(lr-*ofn+1);
			safe_strncpy(t,*ofn,lr-*ofn+1);
			*ofn=t;
			continue;
		case '<':
			if (*ifn) free(*ifn);
			lr=ltrim(lr);
			*ifn=lr;
			while (*lr && *lr!=' ' && *lr!='>' && *lr != '|') lr++;
			if((*ifn != lr) && (lr[-1] == ':')) lr[-1] = 0;
//			if(*lr && *(lr+1))
//				*lr++=0;
//			else
//				*lr=0;
			t = (char*)malloc(lr-*ifn+1);
			safe_strncpy(t,*ifn,lr-*ifn+1);
			*ifn=t;
			continue;
		case '|':
			ch=0;
			num++;
		}
		*lw++=ch;
	}
	*lw=0;
	return num;
}

void DOS_Shell::ParseLine(char * line) {
	LOG(LOG_EXEC,LOG_ERROR)("Parsing command line: %s",line);
	/* Check for a leading @ */
 	if (line[0] == '@') line[0] = ' ';
	line = trim(line);

	/* Do redirection and pipe checks */

	char * in  = 0;
	char * out = 0;

	Bit16u dummy,dummy2;
	Bit32u bigdummy = 0;
	Bitu num = 0;		/* Number of commands in this line */
	bool append;
	bool normalstdin  = false;	/* wether stdin/out are open on start. */
	bool normalstdout = false;	/* Bug: Assumed is they are "con"      */

	num = GetRedirection(line,&in, &out,&append);
	if (num>1) LOG_MSG("SHELL: Multiple command on 1 line not supported");
	if (in || out) {
		normalstdin  = (psp->GetFileHandle(0) != 0xff);
		normalstdout = (psp->GetFileHandle(1) != 0xff);
	}
	if (in) {
		if(DOS_OpenFile(in,OPEN_READ,&dummy)) {	//Test if file exists
			DOS_CloseFile(dummy);
			LOG_MSG("SHELL: Redirect input from %s",in);
			if(normalstdin) DOS_CloseFile(0);	//Close stdin
			DOS_OpenFile(in,OPEN_READ,&dummy);	//Open new stdin
		}
	}
	if (out){
		LOG_MSG("SHELL: Redirect output to %s",out);
		if(normalstdout) DOS_CloseFile(1);
		if(!normalstdin && !in) DOS_OpenFile("con",OPEN_READWRITE,&dummy);
		bool status = true;
		/* Create if not exist. Open if exist. Both in read/write mode */
		if(append) {
			if( (status = DOS_OpenFile(out,OPEN_READWRITE,&dummy)) ) {
				 DOS_SeekFile(1,&bigdummy,DOS_SEEK_END);
			} else {
				status = DOS_CreateFile(out,DOS_ATTR_ARCHIVE,&dummy);	//Create if not exists.
			}
		} else {
			status = DOS_OpenFileExtended(out,OPEN_READWRITE,DOS_ATTR_ARCHIVE,0x12,&dummy,&dummy2);
		}

		if(!status && normalstdout) DOS_OpenFile("con",OPEN_READWRITE,&dummy); //Read only file, open con again
		if(!normalstdin && !in) DOS_CloseFile(0);
	}
	/* Run the actual command */
	DoCommand(line);
	/* Restore handles */
	if(in) {
		DOS_CloseFile(0);
		if(normalstdin) DOS_OpenFile("con",OPEN_READWRITE,&dummy);
		free(in);
	}
	if(out) {
		DOS_CloseFile(1);
		if(!normalstdin) DOS_OpenFile("con",OPEN_READWRITE,&dummy);
		if(normalstdout) DOS_OpenFile("con",OPEN_READWRITE,&dummy);
		if(!normalstdin) DOS_CloseFile(0);
		free(out);
	}
}



void DOS_Shell::RunInternal(void) {
	char input_line[CMD_MAXLINE] = {0};
	while (bf) {
		if (bf->ReadLine(input_line)) {
			if (echo) {
				if (input_line[0] != '@') {
					ShowPrompt();
					WriteOut_NoParsing(input_line);
					WriteOut_NoParsing("\n");
				}
			}
			ParseLine(input_line);
			if (echo) WriteOut_NoParsing("\n");
		}
	}
}

void DOS_Shell::Run(void) {
	char input_line[CMD_MAXLINE] = {0};
	std::string line;
	if (cmd->FindStringRemainBegin("/C",line)) {
		strcpy(input_line,line.c_str());
		char* sep = strpbrk(input_line,"\r\n"); //GTA installer
		if (sep) *sep = 0;
		DOS_Shell temp;
		temp.echo = echo;
		temp.ParseLine(input_line);		//for *.exe *.com  |*.bat creates the bf needed by runinternal;
		temp.RunInternal();				// exits when no bf is found.
		return;
	}
	/* Start a normal shell and check for a first command init */
	if (cmd->FindString("/INIT",line,true)) {
		WriteOut(MSG_Get("SHELL_STARTUP_BEGIN"),VERSION);
#if C_DEBUG
		WriteOut(MSG_Get("SHELL_STARTUP_DEBUG"));
#endif
		if (machine == MCH_CGA) WriteOut(MSG_Get("SHELL_STARTUP_CGA"));
		if (machine == MCH_HERC) WriteOut(MSG_Get("SHELL_STARTUP_HERC"));
		WriteOut(MSG_Get("SHELL_STARTUP_END"));

		strcpy(input_line,line.c_str());
		line.erase();
		ParseLine(input_line);
	} else {
		WriteOut(MSG_Get("SHELL_STARTUP_SUB"),VERSION);
	}
	do {
		if (bf){
			if(bf->ReadLine(input_line)) {
				if (echo) {
					if (input_line[0]!='@') {
						ShowPrompt();
						WriteOut_NoParsing(input_line);
						WriteOut_NoParsing("\n");
					};
				};
				ParseLine(input_line);
				if (echo) WriteOut("\n");
			}

			//DBP: Avoid freeze during infinite loop in batch
			if (last_pic_ticks != (Bit32u)PIC_Ticks)
			{
				last_pic_ticks = (Bit32u)PIC_Ticks;
				same_tick_lines = 0;
			}
			else if (same_tick_lines++ > 250)
			{
				CALLBACK_Idle();
				CALLBACK_Idle();
				last_pic_ticks = PIC_Ticks;
			}
		} else {
			if (echo) ShowPrompt();
			InputCommand(input_line);
			ParseLine(input_line);
			if (echo && !bf) WriteOut_NoParsing("\n");
		}
	} while (!exit && !first_shell->exit);
}

void DOS_Shell::SyntaxError(void) {
	WriteOut(MSG_Get("SHELL_SYNTAXERROR"));
}

class AUTOEXEC:public Module_base {
private:
#ifdef C_DBP_NATIVE_CONFIGFILE
	AutoexecObject autoexec[17];
#else
	AutoexecObject autoexec[1];
#endif
	AutoexecObject autoexec_echo;
public:
	AUTOEXEC(Section* configuration):Module_base(configuration) {
		/* Register a virtual AUOEXEC.BAT file */
		std::string line;
		Section_line * section=static_cast<Section_line *>(configuration);

#ifdef C_DBP_NATIVE_CONFIGFILE
		/* Check -securemode switch to disable mount/imgmount/boot after running autoexec.bat */
		bool secure = control->cmdline->FindExist("-securemode",true);

		/* add stuff from the configfile unless -noautexec or -securemode is specified. */
		char * extra = const_cast<char*>(section->data.c_str());
		if (extra && !secure && !control->cmdline->FindExist("-noautoexec",true)) {
#else
		char * extra = const_cast<char*>(section->data.c_str());
		if (extra) {
#endif
			/* detect if "echo off" is the first line */
			size_t firstline_length = strcspn(extra,"\r\n");
			bool echo_off  = !strncasecmp(extra,"echo off",8);
			if (echo_off && firstline_length == 8) extra += 8;
			else {
				echo_off = !strncasecmp(extra,"@echo off",9);
				if (echo_off && firstline_length == 9) extra += 9;
				else echo_off = false;
			}

			/* if "echo off" move it to the front of autoexec.bat */
			if (echo_off)  { 
				autoexec_echo.InstallBefore("@echo off");
				if (*extra == '\r') extra++; //It can point to \0
				if (*extra == '\n') extra++; //same
			}

			/* Install the stuff from the configfile if anything left after moving echo off */

			if (*extra) autoexec[0].Install(std::string(extra));
		}

#ifdef C_DBP_NATIVE_CONFIGFILE
		/* Check to see for extra command line options to be added (before the command specified on commandline) */
		/* Maximum of extra commands: 10 */
		Bitu i = 1;
		while (control->cmdline->FindString("-c",line,true) && (i <= 11)) {
#ifdef C_DBP_USE_SDL
#if defined (WIN32) || defined (OS2)
			//replace single with double quotes so that mount commands can contain spaces
			for(Bitu temp = 0;temp < line.size();++temp) if(line[temp] == '\'') line[temp]='\"';
#endif //Linux users can simply use \" in their shell
#else
			// command line is not from native process, no conversion needed
#endif
			autoexec[i++].Install(line);
		}

		/* Check for the -exit switch which causes dosbox to when the command on the commandline has finished */
		bool addexit = control->cmdline->FindExist("-exit",true);

		/* Check for first command being a directory or file */
		char buffer[CROSS_LEN+1];
		char orig[CROSS_LEN+1];
		char cross_filesplit[2] = {CROSS_FILESPLIT , 0};

		Bitu dummy = 1;
		bool command_found = false;
		while (control->cmdline->FindCommand(dummy++,line) && !command_found) {
			struct stat test;
			if (line.length() > CROSS_LEN) continue;
			strcpy(buffer,line.c_str());
			if (stat(buffer,&test)) {
				if (getcwd(buffer,CROSS_LEN) == NULL) continue;
				if (strlen(buffer) + line.length() + 1 > CROSS_LEN) continue;
				strcat(buffer,cross_filesplit);
				strcat(buffer,line.c_str());
				if (stat(buffer,&test)) continue;
			}
			if (test.st_mode & S_IFDIR) {
				autoexec[12].Install(std::string("MOUNT C \"") + buffer + "\"");
				autoexec[13].Install("C:");
				if(secure) autoexec[14].Install("z:\\config.com -securemode");
				command_found = true;
			} else {
				char* name = strrchr(buffer,CROSS_FILESPLIT);
				if (!name) { //Only a filename
					line = buffer;
					if (getcwd(buffer,CROSS_LEN) == NULL) continue;
					if (strlen(buffer) + line.length() + 1 > CROSS_LEN) continue;
					strcat(buffer,cross_filesplit);
					strcat(buffer,line.c_str());
					if(stat(buffer,&test)) continue;
					name = strrchr(buffer,CROSS_FILESPLIT);
					if(!name) continue;
				}
				*name++ = 0;
				if (access(buffer,F_OK)) continue;
				autoexec[12].Install(std::string("MOUNT C \"") + buffer + "\"");
				autoexec[13].Install("C:");
				/* Save the non-modified filename (so boot and imgmount can use it (long filenames, case sensivitive)) */
				strcpy(orig,name);
				upcase(name);
				if(strstr(name,".BAT") != 0) {
					if(secure) autoexec[14].Install("z:\\config.com -securemode");
					/* BATch files are called else exit will not work */
					autoexec[15].Install(std::string("CALL ") + name);
					if(addexit) autoexec[16].Install("exit");
				} else if((strstr(name,".IMG") != 0) || (strstr(name,".IMA") !=0 )) {
					//No secure mode here as boot is destructive and enabling securemode disables boot
					/* Boot image files */
					autoexec[15].Install(std::string("BOOT ") + orig);
				} else if((strstr(name,".ISO") != 0) || (strstr(name,".CUE") !=0 )) {
					/* imgmount CD image files */
					/* securemode gets a different number from the previous branches! */
					autoexec[14].Install(std::string("IMGMOUNT D \"") + orig + std::string("\" -t iso"));
					//autoexec[16].Install("D:");
					if(secure) autoexec[15].Install("z:\\config.com -securemode");
					/* Makes no sense to exit here */
				} else {
					if(secure) autoexec[14].Install("z:\\config.com -securemode");
					autoexec[15].Install(name);
					if(addexit) autoexec[16].Install("exit");
				}
				command_found = true;
			}
		}

		/* Combining -securemode, noautoexec and no parameters leaves you with a lovely Z:\. */
		if ( !command_found ) {
			if ( secure ) autoexec[12].Install("z:\\config.com -securemode");
		}
#endif
		VFILE_Register("AUTOEXEC.BAT",(Bit8u *)autoexec_data,(Bit32u)strlen(autoexec_data));
	}
};

static AUTOEXEC* test;

//DBP: memory cleanup
static void AUTOEXEC_ShutDown(Section* /*sec*/) {
	delete test;
	VFILE_Remove("AUTOEXEC.BAT");
}

void AUTOEXEC_Init(Section * sec) {
	test = new AUTOEXEC(sec);
	sec->AddDestroyFunction(&AUTOEXEC_ShutDown);
}

static Bitu INT2E_Handler(void) {
	/* Save return address and current process */
	RealPt save_ret=real_readd(SegValue(ss),reg_sp);
	Bit16u save_psp=dos.psp();

	/* Set first shell as process and copy command */
	dos.psp(DOS_FIRST_SHELL);
	DOS_PSP psp(DOS_FIRST_SHELL);
	psp.SetCommandTail(RealMakeSeg(ds,reg_si));
	SegSet16(ss,RealSeg(psp.GetStack()));
	reg_sp=2046;

	/* Read and fix up command string */
	CommandTail tail;
	MEM_BlockRead(PhysMake(dos.psp(),128),&tail,128);
	if (tail.count<127) tail.buffer[tail.count]=0;
	else tail.buffer[126]=0;
	char* crlf=strpbrk(tail.buffer,"\r\n");
	if (crlf) *crlf=0;

	/* Execute command */
	if (strlen(tail.buffer)) {
		DOS_Shell temp;
		temp.ParseLine(tail.buffer);
		temp.RunInternal();
	}

	/* Restore process and "return" to caller */
	dos.psp(save_psp);
	SegSet16(cs,RealSeg(save_ret));
	reg_ip=RealOff(save_ret);
	reg_ax=0;
	return CBRET_NONE;
}

static char const * const path_string="PATH=Z:\\";
static char const * const comspec_string="COMSPEC=Z:\\COMMAND.COM";
static char const * const full_name="Z:\\COMMAND.COM";
#ifdef C_DBP_LIBRETRO
static char const * const init_line="/INIT Z:\\AUTOEXEC.BAT";
#else
static char const * const init_line="/INIT AUTOEXEC.BAT";
#endif

void SHELL_Init() {
	/* Add messages */
MSG_Add("SHELL_ILLEGAL_PATH","Caminho n„o encontrado.\n");
	MSG_Add("SHELL_CMD_HELP","Se quiser uma lista de todos os comandos internos suportados digite \033[33;1mhelp /all\033[0m.\nVocˆ tamb‚m pode encontrar comandos externos e programas na unidade Z:\nUma breve lista dos comandos mais usados:\n");
	MSG_Add("SHELL_CMD_ECHO_ON","ECHO est  ativado.\n");
	MSG_Add("SHELL_CMD_ECHO_OFF","ECHO est  desativado.\n");
	MSG_Add("SHELL_ILLEGAL_SWITCH","Parƒmetro inv lido - %s.\n");
	MSG_Add("SHELL_MISSING_PARAMETER","Um parƒmetro obrigat¢rio est  faltando.\n");
	MSG_Add("SHELL_CMD_CHDIR_ERROR","Pasta inv lida - %s.\n");
	MSG_Add("SHELL_CMD_CHDIR_HINT","Dica: Para mudar para um tipo de unidade diferente \033[31m%c:\033[0m\n");
	MSG_Add("SHELL_CMD_CHDIR_HINT_2","nome_de_pasta contem espa‡os sem aspas.\nTente \033[31mcd %s\033[0m ou adicione corretamente as aspas.\n");
	MSG_Add("SHELL_CMD_CHDIR_HINT_3","Vocˆ ainda est  no drive Z:, e a pasta especificada n„o pode ser encontrada.\nPara acessar uma unidade montada, mude para a unidade com uma sintaxe como \033[31mC:\033[0m.\n");
	MSG_Add("SHELL_CMD_DATE_HELP","Exibe ou define a data.\n");
	MSG_Add("SHELL_CMD_DATE_ERROR","A data especificada n„o ‚ correta.\n");
	MSG_Add("SHELL_CMD_DATE_DAYS","3DomSegTerQuaQuiSexSab");
	MSG_Add("SHELL_CMD_DATE_NOW","Data atual: ");
	MSG_Add("SHELL_CMD_DATE_SETHLP","Digite a nova data.\n");
	MSG_Add("SHELL_CMD_DATE_FORMAT","M/D/Y");
	MSG_Add("SHELL_CMD_DATE_HELP_LONG","DATE [[/T] [/H] [/S] | data]\n"\
									"  data: Nova data a ser definida\n"\
									"  /S:         Usa a data e hora do sistema anfitri„o no DOS\n"\
									"  /F:         Retorna para a data e hora interna do MS-Dos\n"\
									"  /T:         Exibe apenas a data, sem solicitar uma nova\n"\
									"  /H:         Sincronizar com o anfitri„o\n");
	MSG_Add("SHELL_CMD_TIME_HELP","Exibe ou ajusta a hora do sistema.\n");
	MSG_Add("SHELL_CMD_TIME_NOW","Hora atual: ");
	MSG_Add("SHELL_CMD_TIME_HELP_LONG","TIME [/T] [/H]\n"\
									"  /T:         Exibe apenas a hora, sem solicitar uma nova\n"\
									"  /H:         Sincronizar com o anfitri„o\n");
	MSG_Add("SHELL_CMD_MKDIR_ERROR","N„o ‚ poss¡vel criar a pasta - %s\n");
	MSG_Add("SHELL_CMD_RMDIR_ERROR","Caminho inv lido, n„o ‚ uma pasta ou n„o est  vazia - %s.\n");
	MSG_Add("SHELL_CMD_DEL_ERROR","N„o ‚ poss¡vel excluir - %s.\n");
	MSG_Add("SHELL_SYNTAXERROR","Erro de sintaxe.\n");
	MSG_Add("SHELL_CMD_SET_NOT_SET","Vari vel de ambiente %s n„o definida.\n");
	MSG_Add("SHELL_CMD_SET_OUT_OF_SPACE","N„o h  espa‡o suficiente no ambiente.\n");
	MSG_Add("SHELL_CMD_IF_EXIST_MISSING_FILENAME","IF EXIST: Arquivo inexistente.\n");
	MSG_Add("SHELL_CMD_IF_ERRORLEVEL_MISSING_NUMBER","IF ERRORLEVEL: Falta n£mero.\n");
	MSG_Add("SHELL_CMD_IF_ERRORLEVEL_INVALID_NUMBER","IF ERRORLEVEL: N£mero inv lido.\n");
	MSG_Add("SHELL_CMD_GOTO_MISSING_LABEL","Nenhum r¢tulo fornecido ao comando GOTO.\n");
	MSG_Add("SHELL_CMD_GOTO_LABEL_NOT_FOUND","GOTO: R¢tulo %s n„o encontrado.\n");
	MSG_Add("SHELL_CMD_FILE_NOT_FOUND","Arquivo n„o encontrado - %s.\n");
	MSG_Add("SHELL_CMD_FILE_EXISTS","O arquivo %s j  existe.\n");
	MSG_Add("SHELL_CMD_DIR_INTRO","Pasta de %s.\n");
	MSG_Add("SHELL_CMD_DIR_BYTES_USED","%5d arquivo(s) %17s bytes.\n");
	MSG_Add("SHELL_CMD_DIR_BYTES_FREE","%5d pasta(s)  %17s bytes dispon¡veis.\n");
#ifdef C_DBP_ENABLE_INTROPROGRAM
	MSG_Add("SHELL_EXECUTE_DRIVE_NOT_FOUND","Unidade %c n„o existe!\nVocˆ deve \033[31mmont -la\033[0m antes. Digite \033[1;33mintro\033[0m ou \033[1;33mintro mount\033[0m para mais informa‡„o.\n");
#else
	MSG_Add("SHELL_EXECUTE_DRIVE_NOT_FOUND","Unidade %c n„o existe!\nVocˆ deve \033[31mmont -la\033[0m antes. Digite \033[1;33mintro\033[0m ou \033[1;33mintro mount\033[0m para mais informa‡„o.\n");
#endif
	MSG_Add("SHELL_EXECUTE_ILLEGAL_COMMAND","Comando ou nome de arquivo inv lido - %s.\n");
	MSG_Add("SHELL_CMD_PAUSE","Pressione qualquer tecla para continuar. . .\n");
	MSG_Add("SHELL_CMD_PAUSE_HELP","Suspende o processamento de um arquivo em lotes e exibe uma mensagem.\n");
	MSG_Add("SHELL_CMD_COPY_FAILURE","Erro ao copiar: %s.\n");
	MSG_Add("SHELL_CMD_COPY_SUCCESS","   %d arquivo(s) copiado(s).\n");
	MSG_Add("SHELL_CMD_SUBST_NO_REMOVE","N„o foi poss¡vel remover, a unidade n„o est  em uso.\n");
	MSG_Add("SHELL_CMD_SUBST_FAILURE","SUBST: Existe um erro na sua linha de comando");

	MSG_Add("SHELL_STARTUP_BEGIN", "");
	MSG_Add("SHELL_STARTUP_CGA", "");
	MSG_Add("SHELL_STARTUP_HERC", "");
#if C_DEBUG
	MSG_Add("SHELL_STARTUP_DEBUG", "");
#endif
	MSG_Add("SHELL_STARTUP_END", "");
	MSG_Add("SHELL_STARTUP_SUB", "");

	MSG_Add("SHELL_CMD_CHDIR_HELP","Exibe o nome da pasta ou altera a pasta atual.\n");
	MSG_Add("SHELL_CMD_CHDIR_HELP_LONG","CHDIR [unidade:][caminho]\n"
	        "CHDIR [..]\n"
	        "CD [unidade:][caminho]\n"
	        "CD [..]\n\n"
	        "  ..   Especifica que vocˆ quer ir para a pasta pai.\n\n"
	        "Digite CD unidade: para exibir a pasta atual na unidade especificada.\n"
	        "Digite CD sem parƒmetros para exibir a unidade e pasta atuais.\n");
	MSG_Add("SHELL_CMD_CLS_HELP","Limpa a tela.\n");
	MSG_Add("SHELL_CMD_DIR_HELP","Exibe uma lista de arquivos e subpastas em uma pasta.\n");
	MSG_Add("SHELL_CMD_ECHO_HELP","Exibe mensagens ou ativa ou desativa o eco de comando.\n");
	MSG_Add("SHELL_CMD_EXIT_HELP","Encerra o interpretador de comando.\n");
	MSG_Add("SHELL_CMD_HELP_HELP","Fornece informa‡”es de ajuda sobre comandos do MS-Dos.\n");
	MSG_Add("SHELL_CMD_MKDIR_HELP","Cria uma pasta.\n");
	MSG_Add("SHELL_CMD_MKDIR_HELP_LONG","MKDIR [unidade:][caminho]\n"
	        "MD [unidade:][caminho]\n");
	MSG_Add("SHELL_CMD_RMDIR_HELP","Remove uma pasta.\n");
	MSG_Add("SHELL_CMD_RMDIR_HELP_LONG","RMDIR [unidade:][caminho]\n"
	        "RD [unidade:][caminho]\n");
	MSG_Add("SHELL_CMD_SET_HELP","Exibe, define ou remove vari veis de ambiente.\n");
	MSG_Add("SHELL_CMD_IF_HELP","Realiza processamento condicional em arquivos em lotes.\n");
	MSG_Add("SHELL_CMD_GOTO_HELP","Desvia o interpretador de comandos para uma linha com um r¢tulo em um\n"
		   "arquivo em lotes.\n");
	MSG_Add("SHELL_CMD_SHIFT_HELP","Altera a posi‡„o dos parƒmetros substitu¡veis em um arquivo em lotes.\n");
	MSG_Add("SHELL_CMD_TYPE_HELP","Exibe o conte£do de um arquivo de texto.\n");
	MSG_Add("SHELL_CMD_TYPE_HELP_LONG","TYPE [unidade:][caminho][arquivo]\n");
	MSG_Add("SHELL_CMD_REM_HELP","Registra coment rios em um arquivo em lotes.\n");
	MSG_Add("SHELL_CMD_REM_HELP_LONG","REM [coment rio]\n");
	MSG_Add("SHELL_CMD_NO_WILD","Esta ‚ uma vers„o simples do comando, sem curingas permitidos!\n");
	MSG_Add("SHELL_CMD_RENAME_HELP","Renomeia um ou mais arquivos.\n");
	MSG_Add("SHELL_CMD_RENAME_HELP_LONG","RENAME [unidade:][caminho][nome_de_pasta1 | arquivo1] [nome_de_pasta2 | arquivo2]\n"
	        "REN [unidade:][caminho][nome_de_pasta1 | arquivo1] [nome_de_pasta2 | arquivo2]\n\n"
	        "Note que vocˆ n„o pode especificar uma nova unidade ou caminho para o\n"
			"arquivo de destino.\n\n");
	MSG_Add("SHELL_CMD_DELETE_HELP","Exclui um ou mais arquivos.\n");
	MSG_Add("SHELL_CMD_COPY_HELP","Copia um ou mais arquivos para outro local.\n");
	MSG_Add("SHELL_CMD_CALL_HELP","Chama um arquivo em lotes de outro.\n");
	MSG_Add("SHELL_CMD_SUBST_HELP","Associa um caminho a uma letra de unidade.\n");
#ifdef C_DBP_LIBRETRO //DBP: Added a fully featured implementation of SUBST that supports any source drive
	MSG_Add("SHELL_CMD_SUBST_HELP_LONG", 
	        "  X: [Y:]PATH  -  Associa CAMINHO de Y: … unidade X:.\n"
	        "  X: /D  -  Exclui a unidade substitu¡da X:.\n");
#endif
	MSG_Add("SHELL_CMD_LOADHIGH_HELP","Carrega um programa com mais mem¢ria (requer xms=true,umb=true).\n");
	MSG_Add("SHELL_CMD_CHOICE_HELP","Espera o apertar de uma tecla e define o valor de ERRORLEVEL.\n");
	MSG_Add("SHELL_CMD_CHOICE_HELP_LONG","CHOICE [/C:escolhas] [/N] [/S] texto\n"
	        "  /C[:]escolhas - Especifica as teclas permitidas. Padr„o: yn..\n"
	        "  /N            - N„o mostrar as escolhas no final do prompt.\n"
	        "  /S            - Ativa op‡”es de escolhas sens¡veis a mai£sculas e min£sculas.\n"
	        "  texto         -   Texto a ser mostrado como prompt.\n");
	MSG_Add("SHELL_CMD_ATTRIB_HELP","Exibe ou altera atributos de arquivos.\n");
	MSG_Add("SHELL_CMD_PATH_HELP","Exibe ou define um caminho de pesquisa para arquivos execut veis.\n");
	MSG_Add("SHELL_CMD_VER_HELP","Exibe a vers„o do DOS informada pelo MSDos.\n");
	MSG_Add("SHELL_CMD_VER_VER","DOSBox vers„o %s. Vers„o do DOS informada %d.%02d.\n");

	/* Regular startup */
	call_shellstop=CALLBACK_Allocate();
	/* Setup the startup CS:IP to kill the last running machine when exitted */
	RealPt newcsip=CALLBACK_RealPointer(call_shellstop);
	SegSet16(cs,RealSeg(newcsip));
	reg_ip=RealOff(newcsip);

	CALLBACK_Setup(call_shellstop,shellstop_handler,CB_IRET,"shell stop");
	PROGRAMS_MakeFile("COMMAND.COM",SHELL_ProgramStart);

	/* Now call up the shell for the first time */
	Bit16u psp_seg=DOS_FIRST_SHELL;
	Bit16u env_seg=DOS_FIRST_SHELL+19; //DOS_GetMemory(1+(4096/16))+1;
	Bit16u stack_seg=DOS_GetMemory(2048/16);
	SegSet16(ss,stack_seg);
	reg_sp=2046;

	/* Set up int 24 and psp (Telarium games) */
	real_writeb(psp_seg+16+1,0,0xea);		/* far jmp */
	real_writed(psp_seg+16+1,1,real_readd(0,0x24*4));
	real_writed(0,0x24*4,((Bit32u)psp_seg<<16) | ((16+1)<<4));

	/* Set up int 23 to "int 20" in the psp. Fixes what.exe */
	real_writed(0,0x23*4,((Bit32u)psp_seg<<16));

	/* Set up int 2e handler */
	Bitu call_int2e=CALLBACK_Allocate();
	RealPt addr_int2e=RealMake(psp_seg+16+1,8);
	CALLBACK_Setup(call_int2e,&INT2E_Handler,CB_IRET_STI,Real2Phys(addr_int2e),"Shell Int 2e");
	RealSetVec(0x2e,addr_int2e);

	/* Setup MCBs */
	DOS_MCB pspmcb((Bit16u)(psp_seg-1));
	pspmcb.SetPSPSeg(psp_seg);	// MCB of the command shell psp
	pspmcb.SetSize(0x10+2);
	pspmcb.SetType(0x4d);
	DOS_MCB envmcb((Bit16u)(env_seg-1));
	envmcb.SetPSPSeg(psp_seg);	// MCB of the command shell environment
	envmcb.SetSize(DOS_MEM_START-env_seg);
	envmcb.SetType(0x4d);

	/* Setup environment */
	PhysPt env_write=PhysMake(env_seg,0);
	MEM_BlockWrite(env_write,path_string,(Bitu)(strlen(path_string)+1));
	env_write += (PhysPt)(strlen(path_string)+1);
	MEM_BlockWrite(env_write,comspec_string,(Bitu)(strlen(comspec_string)+1));
	env_write += (PhysPt)(strlen(comspec_string)+1);
	mem_writeb(env_write++,0);
	mem_writew(env_write,1);
	env_write+=2;
	MEM_BlockWrite(env_write,full_name,(Bitu)(strlen(full_name)+1));

	DOS_PSP psp(psp_seg);
	psp.MakeNew(0);
	dos.psp(psp_seg);

	/* The start of the filetable in the psp must look like this:
	 * 01 01 01 00 02
	 * In order to achieve this: First open 2 files. Close the first and
	 * duplicate the second (so the entries get 01) */
	Bit16u dummy=0;
	DOS_OpenFile("CON",OPEN_READWRITE,&dummy);	/* STDIN  */
	DOS_OpenFile("CON",OPEN_READWRITE,&dummy);	/* STDOUT */
	DOS_CloseFile(0);							/* Close STDIN */
	DOS_ForceDuplicateEntry(1,0);				/* "new" STDIN */
	DOS_ForceDuplicateEntry(1,2);				/* STDERR */
	DOS_OpenFile("CON",OPEN_READWRITE,&dummy);	/* STDAUX */
	DOS_OpenFile("PRN",OPEN_READWRITE,&dummy);	/* STDPRN */

	/* Create appearance of handle inheritance by first shell */
	for (Bit16u i=0;i<5;i++) {
		Bit8u handle=psp.GetFileHandle(i);
		if (Files[handle]) Files[handle]->AddRef();
	}

	psp.SetParent(psp_seg);
	/* Set the environment */
	psp.SetEnvironment(env_seg);
	/* Set the command line for the shell start up */
	CommandTail tail;
	tail.count=(Bit8u)strlen(init_line);
	memset(&tail.buffer,0,127);
	strcpy(tail.buffer,init_line);
	MEM_BlockWrite(PhysMake(psp_seg,128),&tail,128);

	/* Setup internal DOS Variables */
	dos.dta(RealMake(psp_seg,0x80));
	dos.psp(psp_seg);


	SHELL_ProgramStart_First_shell(&first_shell);
	first_shell->Run();
	delete first_shell;
	first_shell = 0;//Make clear that it shouldn't be used anymore
}
