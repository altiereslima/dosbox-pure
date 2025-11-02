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


#include "dosbox.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include "programs.h"
#include "support.h"
#include "drives.h"
#include "cross.h"
#include "regs.h"
#include "callback.h"
#include "cdrom.h"
#include "dos_system.h"
#include "dos_inc.h"
#include "bios.h"
#include "bios_disk.h" 
#include "setup.h"
#include "control.h"
#include "inout.h"
#include "dma.h"


#if defined(OS2)
#define INCL DOSFILEMGR
#define INCL_DOSERRORS
#include "os2.h"
#endif

#if defined(WIN32)
#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT)==S_IFDIR)
#endif
#endif

#if C_DEBUG
Bitu DEBUG_EnableDebugger(void);
#endif

void MSCDEX_SetCDInterface(int intNr, int forceCD);
static Bitu ZDRIVE_NUM = 25;

static const char* UnmountHelper(char umount) {
	int i_drive;
	if (umount < '0' || umount > 3+'0')
		i_drive = toupper(umount) - 'A';
	else
		i_drive = umount - '0';

	if (i_drive >= DOS_DRIVES || i_drive < 0)
		return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

	if (i_drive < MAX_DISK_IMAGES && Drives[i_drive] == NULL && imageDiskList[i_drive] == NULL)
		return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

	if (i_drive >= MAX_DISK_IMAGES && Drives[i_drive] == NULL)
		return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

	if (Drives[i_drive]) {
		#ifdef C_DBP_ENABLE_DRIVE_MANAGER
		switch (DriveManager::UnmountDrive(i_drive)) {
			case 1: return MSG_Get("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL");
			case 2: return MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS");
		}
		Drives[i_drive] = 0;
		mem_writeb(Real2Phys(dos.tables.mediaid)+i_drive*9,0);
		#else
		if (dynamic_cast<Virtual_Drive*>(Drives[i_drive])) return MSG_Get("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL");
		void DBP_Unmount(char drive);
		DBP_Unmount('A' + i_drive);
		DBP_ASSERT(!Drives[i_drive]);
		#endif
		if (i_drive == DOS_GetDefaultDrive()) {
			DOS_SetDrive(ZDRIVE_NUM);
		}

	}

	if (i_drive < MAX_DISK_IMAGES && imageDiskList[i_drive]) {
		delete imageDiskList[i_drive];
		imageDiskList[i_drive] = NULL;
	}

	return MSG_Get("PROGRAM_MOUNT_UMOUNT_SUCCESS");
}

class MOUNT : public Program {
public:
	void Move_Z(char new_z) {
		char newz_drive = (char) toupper(new_z);
		int i_newz = newz_drive - 'A';
		if (i_newz >= 0 && i_newz < DOS_DRIVES-1 && !Drives[i_newz]) {
			ZDRIVE_NUM = i_newz;
			/* remap drives */
			Drives[i_newz] = Drives[25];
			Drives[25] = 0;
			if (!first_shell) return; //Should not be possible			
			/* Update environment */
			std::string line = "";
			char ppp[2] = {newz_drive,0};
			std::string tempenv = ppp; tempenv += ":\\";
			if (first_shell->GetEnvStr("PATH",line)){
				std::string::size_type idx = line.find('=');
				std::string value = line.substr(idx +1 , std::string::npos);
				while ( (idx = value.find("Z:\\")) != std::string::npos ||
					(idx = value.find("z:\\")) != std::string::npos  )
					value.replace(idx,3,tempenv);
				line = value;
			}
			if (!line.size()) line = tempenv;
			first_shell->SetEnv("PATH",line.c_str());
			tempenv += "COMMAND.COM";
			first_shell->SetEnv("COMSPEC",tempenv.c_str());

			/* Update batch file if running from Z: (very likely: autoexec) */
			if(first_shell->bf) {
				std::string &name = first_shell->bf->filename;
				if(name.length() >2 &&  name[0] == 'Z' && name[1] == ':') name[0] = newz_drive;
			}
			/* Change the active drive */
			if (DOS_GetDefaultDrive() == 25) DOS_SetDrive(i_newz);
		}
	}
	void ListMounts(void) {
		char name[DOS_NAMELENGTH_ASCII];Bit32u size;Bit16u date;Bit16u time;Bit8u attr;
		/* Command uses dta so set it to our internal dta */
		RealPt save_dta = dos.dta();
		dos.dta(dos.tables.tempdta);
		DOS_DTA dta(dos.dta());

		WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_1"));
		WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_FORMAT"),"Drive","Type","Label");
		for(int p = 0;p < 8;p++) WriteOut("----------");

		for (int d = 0;d < DOS_DRIVES;d++) {
			if (!Drives[d]) continue;

			char root[7] = {static_cast<char>('A'+d),':','\\','*','.','*',0};
			bool ret = DOS_FindFirst(root,DOS_ATTR_VOLUME);
			if (ret) {
				dta.GetResult(name,size,date,time,attr);
				DOS_FindNext(); //Mark entry as invalid
			} else name[0] = 0;

			/* Change 8.3 to 11.0 */
			char* dot = strchr(name,'.');
			if(dot && (dot - name == 8) ) { 
				name[8] = name[9];name[9] = name[10];name[10] = name[11];name[11] = 0;
			}

			root[1] = 0; //This way, the format string can be reused.
			WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_FORMAT"),root, Drives[d]->GetInfo(),name);		
		}
		dos.dta(save_dta);
	}

	void Run(void) {
		DOS_Drive * newdrive;char drive;
		std::string label;
		std::string umount;
		std::string newz;

		//Hack To allow long commandlines
		ChangeToLongCmd();
		/* Parse the command line */
		/* if the command line is empty show current mounts */
		if (!cmd->GetCount()) {
			ListMounts();
			return;
		}

		/* In secure mode don't allow people to change mount points. 
		 * Neither mount nor unmount */
		if(control->SecureMode()) {
			WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
			return;
		}
#ifdef C_DBP_NATIVE_CONFIGFILE
		bool path_relative_to_last_config = false;
		if (cmd->FindExist("-pr",true)) path_relative_to_last_config = true;
#endif

		/* Check for unmounting */
		if (cmd->FindString("-u",umount,false)) {
			WriteOut(UnmountHelper(umount[0]), toupper(umount[0]));
			return;
		}
		
		/* Check for moving Z: */
		/* Only allowing moving it once. It is merely a convenience added for the wine team */
		if (ZDRIVE_NUM == 25 && cmd->FindString("-z", newz,false)) {
			Move_Z(newz[0]);
			return;
		}

#ifdef C_DBP_USE_SDL
		/* Show list of cdroms */
		if (cmd->FindExist("-cd",false)) {
			int num = SDL_CDNumDrives();
   			WriteOut(MSG_Get("PROGRAM_MOUNT_CDROMS_FOUND"),num);
			for (int i=0; i<num; i++) {
				WriteOut("%2d. %s\n",i,SDL_CDName(i));
			};
			return;
		}
#endif

		std::string type="dir";
		cmd->FindString("-t",type,true);
		bool iscdrom = (type =="cdrom"); //Used for mscdex bug cdrom label name emulation
		if (type=="floppy" || type=="dir" || type=="cdrom"
#ifdef C_DBP_NATIVE_OVERLAY
			|| type =="overlay"
#endif
			) {
			Bit16u sizes[4] ={0};
			Bit8u mediaid;
			std::string str_size = "";
			if (type=="floppy") {
				str_size="512,1,2880,2880";/* All space free */
				mediaid=0xF0;		/* Floppy 1.44 media */
			} else if (type=="dir"
#ifdef C_DBP_NATIVE_OVERLAY
				|| type == "overlay"
#endif
				) {
				// 512*32*32765==~500MB total size
				// 512*32*16000==~250MB total free size
				str_size="512,32,32765,16000";
				mediaid=0xF8;		/* Hard Disk */
			} else if (type=="cdrom") {
				str_size="2048,1,65535,0";
				mediaid=0xF8;		/* Hard Disk */
			} else {
				WriteOut(MSG_Get("PROGAM_MOUNT_ILL_TYPE"),type.c_str());
				return;
			}
			/* Parse the free space in mb's (kb's for floppies) */
			std::string mb_size;
			if(cmd->FindString("-freesize",mb_size,true)) {
				char teststr[1024];
				Bit16u freesize = static_cast<Bit16u>(atoi(mb_size.c_str()));
				if (type=="floppy") {
					// freesize in kb
					sprintf(teststr,"512,1,2880,%d",freesize*1024/(512*1));
				} else {
					Bit32u total_size_cyl=32765;
					Bit32u free_size_cyl=(Bit32u)freesize*1024*1024/(512*32);
					if (free_size_cyl>65534) free_size_cyl=65534;
					if (total_size_cyl<free_size_cyl) total_size_cyl=free_size_cyl+10;
					if (total_size_cyl>65534) total_size_cyl=65534;
					sprintf(teststr,"512,32,%d,%d",total_size_cyl,free_size_cyl);
				}
				str_size=teststr;
			}
		   
			cmd->FindString("-size",str_size,true);
			char number[21] = { 0 };const char * scan = str_size.c_str();
			Bitu index = 0;Bitu count = 0;
			/* Parse the str_size string */
			while (*scan && index < 20 && count < 4) {
				if (*scan==',') {
					number[index] = 0;
					sizes[count++] = atoi(number);
					index = 0;
				} else number[index++] = *scan;
				scan++;
			}
			if (count < 4) {
				number[index] = 0; //always goes correct as index is max 20 at this point.
				sizes[count] = atoi(number);
			}
		
			// get the drive letter
			cmd->FindCommand(1,temp_line);
			if ((temp_line.size() > 2) || ((temp_line.size()>1) && (temp_line[1]!=':'))) goto showusage;
			int i_drive = toupper(temp_line[0]);
			if (!isalpha(i_drive)) goto showusage;
			if ((i_drive - 'A') >= DOS_DRIVES || (i_drive-'A') < 0 ) goto showusage;
			drive = static_cast<char>(i_drive);
#ifdef C_DBP_NATIVE_OVERLAY
			if (type == "overlay") {
				//Ensure that the base drive exists:
				if (!Drives[drive-'A']) {
					WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_NO_BASE"));
					return;
				}
			}
			else
#endif
			if (Drives[drive-'A']) {
				WriteOut(MSG_Get("PROGRAM_MOUNT_ALREADY_MOUNTED"),drive,Drives[drive-'A']->GetInfo());
				return;
			}

			if (!cmd->FindCommand(2,temp_line)) goto showusage;
			if (!temp_line.size()) goto showusage;
#ifdef C_DBP_NATIVE_CONFIGFILE
			if(path_relative_to_last_config && control->configfiles.size() && !Cross::IsPathAbsolute(temp_line)) {
				std::string lastconfigdir(control->configfiles[control->configfiles.size()-1]);
				std::string::size_type pos = lastconfigdir.rfind(CROSS_FILESPLIT);
				if(pos == std::string::npos) pos = 0; //No directory then erase string
				lastconfigdir.erase(pos);
				if (lastconfigdir.length())	temp_line = lastconfigdir + CROSS_FILESPLIT + temp_line;
			}
#endif
#ifndef C_DBP_HAVE_FPATH_NOCASE
			struct stat test;
			//Win32 : strip tailing backslashes
			//os2: some special drive check
			//rest: substitute ~ for home
			bool failed = false;
#if defined (WIN32) || defined(OS2)
			/* Removing trailing backslash if not root dir so stat will succeed */
			if(temp_line.size() > 3 && temp_line[temp_line.size()-1]=='\\') temp_line.erase(temp_line.size()-1,1);
			if (stat(temp_line.c_str(),&test))
#endif
#if defined(WIN32)
// Nothing to do here.
#elif defined (OS2)
			{
				if (temp_line.size() <= 2) // Seems to be a drive.
				{
					failed = true;
					HFILE cdrom_fd = 0;
					ULONG ulAction = 0;

					APIRET rc = DosOpen((unsigned char*)temp_line.c_str(), &cdrom_fd, &ulAction, 0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
						OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0L);
					DosClose(cdrom_fd);
					if (rc != NO_ERROR && rc != ERROR_NOT_READY)
					{
						failed = true;
					} else {
						failed = false;
					}
				}
			}
			if (failed)
#else
			if (stat(temp_line.c_str(),&test)) {
				failed = true;
				Cross::ResolveHomedir(temp_line);
				//Try again after resolving ~
				if(!stat(temp_line.c_str(),&test)) failed = false;
			}
			if(failed)
#endif
			/* IF IS ABOVE INSIDE PREPROCESSOR IF BLOCK */ {
				WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_1"),temp_line.c_str());
				return;
			}
			/* Not a switch so a normal directory/file */
			if (!S_ISDIR(test.st_mode)) {
#ifdef OS2
				HFILE cdrom_fd = 0;
				ULONG ulAction = 0;

				APIRET rc = DosOpen((unsigned char*)temp_line.c_str(), &cdrom_fd, &ulAction, 0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
					OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0L);
				DosClose(cdrom_fd);
				if (rc != NO_ERROR && rc != ERROR_NOT_READY) {
				WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str());
				return;
			}
#else
				WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str());
				return;
#endif
			}
#else
			#ifdef C_DBP_NATIVE_HOMEDIR
			Cross::ResolveHomedir(temp_line);
			#endif
			bool path_is_dir;
			if (!fpath_nocase(temp_line, &path_is_dir)) { WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_1"),temp_line.c_str()); return; }
			if (!path_is_dir) { WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str()); return; }
#endif

			if (temp_line[temp_line.size()-1]!=CROSS_FILESPLIT) temp_line+=CROSS_FILESPLIT;
			Bit8u bit8size=(Bit8u) sizes[1];
			if (type=="cdrom") {
#ifdef C_DBP_NATIVE_CDROM
				int num = -1;
				cmd->FindInt("-usecd",num,true);
				int error = 0;
				if (cmd->FindExist("-aspi",false)) {
					MSCDEX_SetCDInterface(CDROM_USE_ASPI, num);
				} else if (cmd->FindExist("-ioctl_dio",false)) {
					MSCDEX_SetCDInterface(CDROM_USE_IOCTL_DIO, num);
				} else if (cmd->FindExist("-ioctl_dx",false)) {
					MSCDEX_SetCDInterface(CDROM_USE_IOCTL_DX, num);
#if defined (WIN32)
				} else if (cmd->FindExist("-ioctl_mci",false)) {
					MSCDEX_SetCDInterface(CDROM_USE_IOCTL_MCI, num);
#endif
				} else if (cmd->FindExist("-noioctl",false)) {
					MSCDEX_SetCDInterface(CDROM_USE_SDL, num);
				} else {
#if defined (WIN32)
					// Check OS
					OSVERSIONINFO osi;
					osi.dwOSVersionInfoSize = sizeof(osi);
					GetVersionEx(&osi);
					if ((osi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (osi.dwMajorVersion>5)) {
						// Vista/above
						MSCDEX_SetCDInterface(CDROM_USE_IOCTL_DX, num);
					} else {
						MSCDEX_SetCDInterface(CDROM_USE_IOCTL_DIO, num);
					}
#else
					MSCDEX_SetCDInterface(CDROM_USE_IOCTL_DIO, num);
#endif
				}
#else
				int error = 0;
#endif /* C_DBP_NATIVE_CDROM */
				newdrive  = new cdromDrive(drive,temp_line.c_str(),sizes[0],bit8size,sizes[2],0,mediaid,error);
				// Check Mscdex, if it worked out...
				switch (error) {
					case 0  :	WriteOut(MSG_Get("MSCDEX_SUCCESS"));				break;
					case 1  :	WriteOut(MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS"));	break;
					case 2  :	WriteOut(MSG_Get("MSCDEX_ERROR_NOT_SUPPORTED"));	break;
					case 3  :	WriteOut(MSG_Get("MSCDEX_ERROR_PATH"));				break;
					case 4  :	WriteOut(MSG_Get("MSCDEX_TOO_MANY_DRIVES"));		break;
					case 5  :	WriteOut(MSG_Get("MSCDEX_LIMITED_SUPPORT"));		break;
					default :	WriteOut(MSG_Get("MSCDEX_UNKNOWN_ERROR"));			break;
				};
				if (error && error!=5) {
					delete newdrive;
					return;
				}
			} else {
				/* Give a warning when mount c:\ or the / */
#if defined (WIN32) || defined(OS2)
				if( (temp_line == "c:\\") || (temp_line == "C:\\") || 
				    (temp_line == "c:/") || (temp_line == "C:/")    )	
					WriteOut(MSG_Get("PROGRAM_MOUNT_WARNING_WIN"));
#else
				if(temp_line == "/") WriteOut(MSG_Get("PROGRAM_MOUNT_WARNING_OTHER"));
#endif
#ifdef C_DBP_NATIVE_OVERLAY
				if(type == "overlay") {
					localDrive* ldp = dynamic_cast<localDrive*>(Drives[drive-'A']);
					cdromDrive* cdp = dynamic_cast<cdromDrive*>(Drives[drive-'A']);
					if (!ldp || cdp) {
						WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_INCOMPAT_BASE"));
						return;
					}
					std::string base = ldp->getBasedir();
					Bit8u o_error = 0;
					newdrive = new Overlay_Drive(base.c_str(),temp_line.c_str(),sizes[0],bit8size,sizes[2],sizes[3],mediaid,o_error);
					//Erase old drive on success
					if (newdrive) {
						if (o_error) { 
							if (o_error == 1) WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_MIXED_BASE"));
							else if (o_error == 2) WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_SAME_AS_BASE"));
							else WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_GENERIC_ERROR"));
							delete newdrive;
							return;
						}

						//Copy current directory if not marked as deleted.
						if (newdrive->TestDir(ldp->curdir)) {
							strcpy(newdrive->curdir,ldp->curdir);
						}

						delete Drives[drive-'A'];
						Drives[drive-'A'] = 0;
					} else { 
						WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_GENERIC_ERROR"));
						return;
					}
				} else
#endif /* C_DBP_NATIVE_OVERLAY */
				{
					newdrive = new localDrive(temp_line.c_str(),sizes[0],bit8size,sizes[2],sizes[3],mediaid);
				}
			}
		} else {
			WriteOut(MSG_Get("PROGRAM_MOUNT_ILL_TYPE"),type.c_str());
			return;
		}
		if (!newdrive) E_Exit("DOS:Can't create drive");
		Drives[drive-'A']=newdrive;
		/* Set the correct media byte in the table */
		mem_writeb(Real2Phys(dos.tables.mediaid)+(drive-'A')*9,newdrive->GetMediaByte());
#ifdef C_DBP_NATIVE_OVERLAY
		if (type == "overlay") {
			WriteOut(MSG_Get("PROGRAM_MOUNT_OVERLAY_STATUS"),temp_line.c_str(),drive);
		} else
#endif
		{
			WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"),drive,newdrive->GetInfo());
		}
		/* check if volume label is given and don't allow it to updated in the future */
		if (cmd->FindString("-label",label,true)) newdrive->label.SetLabel(label.c_str(),iscdrom,false);
		/* For hard drives set the label to DRIVELETTER_Drive.
		 * For floppy drives set the label to DRIVELETTER_Floppy.
		 * This way every drive except cdroms should get a label.*/
		else if(type == "dir"
#ifdef C_DBP_NATIVE_OVERLAY
			|| type == "overlay"
#endif
			) { 
			label = drive; label += "_DRIVE";
			newdrive->label.SetLabel(label.c_str(),false,false);
		} else if(type == "floppy") {
			label = drive; label += "_FLOPPY";
			newdrive->label.SetLabel(label.c_str(),false,true);
		}
		if(type == "floppy") incrementFDD();
		return;
showusage:
#if defined (WIN32) || defined(OS2)
	   WriteOut(MSG_Get("PROGRAM_MOUNT_USAGE"),"d:\\dosprogs","d:\\dosprogs");
#else
	   WriteOut(MSG_Get("PROGRAM_MOUNT_USAGE"),"~/dosprogs","~/dosprogs");		   
#endif
		return;
	}
};

static void MOUNT_ProgramStart(Program * * make) {
	*make=new MOUNT;
}

class MEM : public Program {
public:
	void Run(void) {
		/* Show conventional Memory */
		WriteOut("\n");

		Bit16u umb_start=dos_infoblock.GetStartOfUMBChain();
		Bit8u umb_flag=dos_infoblock.GetUMBChainState();
		Bit8u old_memstrat=DOS_GetMemAllocStrategy()&0xff;
		if (umb_start!=0xffff) {
			if ((umb_flag&1)==1) DOS_LinkUMBsToMemChain(0);
			DOS_SetMemAllocStrategy(0);
		}

		Bit16u seg,blocks;blocks=0xffff;
		DOS_AllocateMemory(&seg,&blocks);
		WriteOut(MSG_Get("PROGRAM_MEM_CONVEN"),blocks*16/1024);

		if (umb_start!=0xffff) {
			DOS_LinkUMBsToMemChain(1);
			DOS_SetMemAllocStrategy(0x40);	// search in UMBs only

			Bit16u largest_block=0,total_blocks=0,block_count=0;
			for (;; block_count++) {
				blocks=0xffff;
				DOS_AllocateMemory(&seg,&blocks);
				if (blocks==0) break;
				total_blocks+=blocks;
				if (blocks>largest_block) largest_block=blocks;
				DOS_AllocateMemory(&seg,&blocks);
			}

			Bit8u current_umb_flag=dos_infoblock.GetUMBChainState();
			if ((current_umb_flag&1)!=(umb_flag&1)) DOS_LinkUMBsToMemChain(umb_flag);
			DOS_SetMemAllocStrategy(old_memstrat);	// restore strategy

			if (block_count>0) WriteOut(MSG_Get("PROGRAM_MEM_UPPER"),total_blocks*16/1024,block_count,largest_block*16/1024);
		}

		/* Test for and show free XMS */
		reg_ax=0x4300;CALLBACK_RunRealInt(0x2f);
		if (reg_al==0x80) {
			reg_ax=0x4310;CALLBACK_RunRealInt(0x2f);
			Bit16u xms_seg=SegValue(es);Bit16u xms_off=reg_bx;
			reg_ah=8;
			CALLBACK_RunRealFar(xms_seg,xms_off);
			if (!reg_bl) {
				WriteOut(MSG_Get("PROGRAM_MEM_EXTEND"),reg_dx);
			}
		}	
		/* Test for and show free EMS */
		Bit16u handle;
		char emm[9] = { 'E','M','M','X','X','X','X','0',0 };
		if (DOS_OpenFile(emm,0,&handle)) {
			DOS_CloseFile(handle);
			reg_ah=0x42;
			CALLBACK_RunRealInt(0x67);
			WriteOut(MSG_Get("PROGRAM_MEM_EXPAND"),reg_bx*16);
		}
	}
};


static void MEM_ProgramStart(Program * * make) {
	*make=new MEM;
}

extern Bit32u floppytype;


class BOOT : public Program {
private:
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
	DOS_File *getFSFile(char const * filename, Bit32u *ksize, Bit32u *bsize,bool tryload=false) {
		bool writable;
		DOS_File *f = FindAndOpenDosFile(filename, bsize, &writable);
		if (!f)
		{
			WriteOut(MSG_Get("PROGRAM_BOOT_NOT_EXIST"));
			return NULL;
		}
		if (!writable)
			WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
		*ksize = (*bsize / 1024);
		return f;
	}
#else /*C_DBP_SUPPORT_DISK_MOUNT_DOSFILE*/
   
	FILE *getFSFile_mounted(char const* filename, Bit32u *ksize, Bit32u *bsize, Bit8u *error) {
		//if return NULL then put in error the errormessage code if an error was requested
		bool tryload = (*error)?true:false;
		*error = 0;
		Bit8u drive;
		FILE *tmpfile;
		char fullname[DOS_PATHLENGTH];

		localDrive* ldp=0;
		if (!DOS_MakeName(const_cast<char*>(filename),fullname,&drive)) return NULL;

#ifdef C_DBP_ENABLE_EXCEPTIONS //this try catch is meaningless anyway, nothing in it throws
		try {		
#endif
			ldp=dynamic_cast<localDrive*>(Drives[drive]);
			if(!ldp) return NULL;

			tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
			if(tmpfile == NULL) {
				if (!tryload) *error=1;
				return NULL;
			}

			// get file size
			fseek(tmpfile,0L, SEEK_END);
			*ksize = (ftell(tmpfile) / 1024);
			*bsize = ftell(tmpfile);
			fclose(tmpfile);

			tmpfile = ldp->GetSystemFilePtr(fullname, "rb+");
			if(tmpfile == NULL) {
//				if (!tryload) *error=2;
//				return NULL;
				WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
				tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
				if(tmpfile == NULL) {
					if (!tryload) *error=1;
					return NULL;
				}
			}

			return tmpfile;
#ifdef C_DBP_ENABLE_EXCEPTIONS
		}
		catch(...) {
			return NULL;
		}
#endif
	}
   
	FILE *getFSFile(char const * filename, Bit32u *ksize, Bit32u *bsize,bool tryload=false) {
		Bit8u error = tryload?1:0;
		FILE* tmpfile = getFSFile_mounted(filename,ksize,bsize,&error);
		if(tmpfile) return tmpfile;
		//File not found on mounted filesystem. Try regular filesystem
		std::string filename_s(filename);
		Cross::ResolveHomedir(filename_s);
		tmpfile = fopen_wrap(filename_s.c_str(),"rb+");
		if(!tmpfile) {
			if( (tmpfile = fopen_wrap(filename_s.c_str(),"rb")) ) {
				//File exists; So can't be opened in correct mode => error 2
//				fclose(tmpfile);
//				if(tryload) error = 2;
				WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
				fseek(tmpfile,0L, SEEK_END);
				*ksize = (ftell(tmpfile) / 1024);
				*bsize = ftell(tmpfile);
				return tmpfile;
			}
			// Give the delayed errormessages from the mounted variant (or from above)
			if(error == 1) WriteOut(MSG_Get("PROGRAM_BOOT_NOT_EXIST"));
			if(error == 2) WriteOut(MSG_Get("PROGRAM_BOOT_NOT_OPEN"));
			return NULL;
		}
		fseek(tmpfile,0L, SEEK_END);
		*ksize = (ftell(tmpfile) / 1024);
		*bsize = ftell(tmpfile);
		return tmpfile;
	}
#endif /*C_DBP_SUPPORT_DISK_MOUNT_DOSFILE*/

	void printError(void) {
		WriteOut(MSG_Get("PROGRAM_BOOT_PRINT_ERROR"));
	}

	void disable_umb_ems_xms(void) {
		Section* dos_sec = control->GetSection("dos");
		dos_sec->ExecuteDestroy(false);
		char test[20];
		strcpy(test,"umb=false");
		dos_sec->HandleInputline(test);
		strcpy(test,"xms=false");
		dos_sec->HandleInputline(test);
		strcpy(test,"ems=false");
		dos_sec->HandleInputline(test);
#ifdef C_DBP_LIBRETRO
		dos_sec->GetProp("umb")->MarkFixed();
		dos_sec->GetProp("xms")->MarkFixed();
		dos_sec->GetProp("ems")->MarkFixed();
#endif
		dos_sec->ExecuteInit(false);
     }

public:
   
	void Run(void) {
		//Hack To allow long commandlines
		ChangeToLongCmd();
		/* In secure mode don't allow people to boot stuff. 
		 * They might try to corrupt the data on it */
		if(control->SecureMode()) {
			WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
			return;
		}

#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
		imageDisk *usefile_1=NULL;
		imageDisk *usefile_2=NULL;
		std::string first_img_path;
#else
		FILE *usefile_1=NULL;
		FILE *usefile_2=NULL;
#endif
		Bitu i=0; 
		Bit32u floppysize=0;
		Bit32u rombytesize_1=0;
		Bit32u rombytesize_2=0;
		Bit8u drive = 'A';
		std::string cart_cmd="";

		if(!cmd->GetCount()) {
			printError();
			return;
		}
		while(i<cmd->GetCount()) {
			if(cmd->FindCommand(i+1, temp_line)) {
				if((temp_line == "-l") || (temp_line == "-L")) {
					/* Specifying drive... next argument then is the drive */
					i++;
					if(cmd->FindCommand(i+1, temp_line)) {
						drive=toupper(temp_line[0]);
						if ((drive != 'A') && (drive != 'C') && (drive != 'D')) {
							printError();
							return;
						}

					} else {
						printError();
						return;
					}
					i++;
					continue;
				}

				if((temp_line == "-e") || (temp_line == "-E")) {
					/* Command mode for PCJr cartridges */
					i++;
					if(cmd->FindCommand(i + 1, temp_line)) {
						for(size_t ct = 0;ct < temp_line.size();ct++) temp_line[ct] = toupper(temp_line[ct]);
						cart_cmd = temp_line;
					} else {
						printError();
						return;
					}
					i++;
					continue;
				}

#ifdef C_DBP_ENABLE_DISKSWAP
				if (imageDiskList[0] != NULL || imageDiskList[1] != NULL) {
					WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_MOUNTED"));
					return;
				}

				if ( i >= MAX_SWAPPABLE_DISKS ) {
					return; //TODO give a warning.
				}
#endif
				WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_OPEN"), temp_line.c_str());
				Bit32u rombytesize;
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
				DOS_File *usefile = getFSFile(temp_line.c_str(), &floppysize, &rombytesize);
#else
				FILE *usefile = getFSFile(temp_line.c_str(), &floppysize, &rombytesize);
#endif
				if(usefile != NULL) {
#ifdef C_DBP_ENABLE_DISKSWAP
#error Old code, i is probably wrong, should be drive?
					if(diskSwap[i] != NULL) delete diskSwap[i];
					diskSwap[i] = new imageDisk(usefile, temp_line.c_str(), floppysize, false);
					if (usefile_1==NULL) {
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
						first_img_path = temp_line;
						usefile_1=diskSwap[i];
#else
						usefile_1=usefile;
#endif
						rombytesize_1=rombytesize;
					} else {
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
						usefile_2=diskSwap[i];
#else
						usefile_2=usefile;
#endif
						rombytesize_2=rombytesize;
					}
#else
					imageDisk* disk = new imageDisk(usefile, temp_line.c_str(), floppysize, false);
					if (usefile_1==NULL) {
						first_img_path = temp_line;
						usefile_1=disk;
						rombytesize_1=rombytesize;
					} else if (usefile_2==NULL) {
						usefile_2=disk;
						rombytesize_2=rombytesize;
					} else {
						delete disk;
					}
#endif

				} else {
					WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_NOT_OPEN"), temp_line.c_str());
					return;
				}

			}
			i++;
		}

#ifdef C_DBP_ENABLE_DISKSWAP
		swapPosition = 0;

		swapInDisks();
#else
		// assign to image disk list drive if not already mounted same file (which might also have a fatDrive in Drives[] array that is in use)
		if (usefile_1 && (!imageDiskList[drive-65] || strcmp(imageDiskList[drive-65]->diskname, first_img_path.c_str())))
		{
			if (imageDiskList[drive-65]) delete imageDiskList[0];
			imageDiskList[drive-65]=usefile_1;
		}
#endif

		if(imageDiskList[drive-65]==NULL) {
			WriteOut(MSG_Get("PROGRAM_BOOT_UNABLE"), drive);
			return;
		}

		bootSector bootarea;
		imageDiskList[drive-65]->Read_Sector(0,0,1,(Bit8u *)&bootarea);
		if ((bootarea.rawdata[0]==0x50) && (bootarea.rawdata[1]==0x43) && (bootarea.rawdata[2]==0x6a) && (bootarea.rawdata[3]==0x72)) {
			if (machine!=MCH_PCJR) WriteOut(MSG_Get("PROGRAM_BOOT_CART_WO_PCJR"));
			else {
				Bit8u rombuf[65536];
				Bits cfound_at=-1;
				if (cart_cmd!="") {
					/* read cartridge data into buffer */
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
					imageDiskList[drive-65]->Read_Raw(rombuf, 0x200, rombytesize_1-0x200);
#else
					fseek(usefile_1,0x200L, SEEK_SET);
					fread(rombuf, 1, rombytesize_1-0x200, usefile_1);
#endif

					char cmdlist[1024];
					cmdlist[0]=0;
					Bitu ct=6;
					Bits clen=rombuf[ct];
					char buf[257];
					if (cart_cmd=="?") {
						while (clen!=0) {
							strncpy(buf,(char*)&rombuf[ct+1],clen);
							buf[clen]=0;
							upcase(buf);
							strcat(cmdlist," ");
							strcat(cmdlist,buf);
							ct+=1+clen+3;
							if (ct>sizeof(cmdlist)) break;
							clen=rombuf[ct];
						}
						if (ct>6) {
							WriteOut(MSG_Get("PROGRAM_BOOT_CART_LIST_CMDS"),cmdlist);
						} else {
							WriteOut(MSG_Get("PROGRAM_BOOT_CART_NO_CMDS"));
						}
#ifdef C_DBP_ENABLE_DISKSWAP
						for(Bitu dct=0;dct<MAX_SWAPPABLE_DISKS;dct++) {
							if(diskSwap[dct]!=NULL) {
								delete diskSwap[dct];
								diskSwap[dct]=NULL;
							}
						}
						//fclose(usefile_1); //delete diskSwap closes the file
#else
						if (usefile_1) delete usefile_1; // clears imageDiskList[drive-65] if needed
						if (usefile_2) delete usefile_2;
#endif
						return;
					} else {
						while (clen!=0) {
							strncpy(buf,(char*)&rombuf[ct+1],clen);
							buf[clen]=0;
							upcase(buf);
							strcat(cmdlist," ");
							strcat(cmdlist,buf);
							ct+=1+clen;

							if (cart_cmd==buf) {
								cfound_at=ct;
								break;
							}

							ct+=3;
							if (ct>sizeof(cmdlist)) break;
							clen=rombuf[ct];
						}
						if (cfound_at<=0) {
							if (ct>6) {
								WriteOut(MSG_Get("PROGRAM_BOOT_CART_LIST_CMDS"),cmdlist);
							} else {
								WriteOut(MSG_Get("PROGRAM_BOOT_CART_NO_CMDS"));
							}
#ifdef C_DBP_ENABLE_DISKSWAP
							for(Bitu dct=0;dct<MAX_SWAPPABLE_DISKS;dct++) {
								if(diskSwap[dct]!=NULL) {
									delete diskSwap[dct];
									diskSwap[dct]=NULL;
								}
							}
							//fclose(usefile_1); //Delete diskSwap closes the file
#else
							if (usefile_1) delete usefile_1; // clears imageDiskList[drive-65] if needed
							if (usefile_2) delete usefile_2;
#endif
							return;
						}
					}
				}

				disable_umb_ems_xms();
				void PreparePCJRCartRom(void);
				PreparePCJRCartRom();

				if (usefile_1==NULL) return;

#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
				DOS_File* tfile = FindAndOpenDosFile("system.rom", NULL, NULL, first_img_path.c_str());
				if (tfile!=NULL) {
					Bit32u seek;
					tfile->Seek(&(seek = 0x3000), DOS_SEEK_SET);
					Bit16u read_size = 0xb000;
					if (tfile->Read(rombuf, &read_size) && read_size == 0xb000) {
						for(i=0;i<0xb000;i++) phys_writeb(0xf3000+i,rombuf[i]);
					}
					tfile->Close();
					delete tfile;
				}
#else
				Bit32u sz1,sz2;
				FILE *tfile = getFSFile("system.rom", &sz1, &sz2, true);
				if (tfile!=NULL) {
					fseek(tfile, 0x3000L, SEEK_SET);
					Bit32u drd=(Bit32u)fread(rombuf, 1, 0xb000, tfile);
					if (drd==0xb000) {
						for(i=0;i<0xb000;i++) phys_writeb(0xf3000+i,rombuf[i]);
					}
					fclose(tfile);
				}
#endif

				if (usefile_2!=NULL) {
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
					usefile_2->Read_Raw(rombuf, 0x0, 0x200);
#else
					fseek(usefile_2, 0x0L, SEEK_SET);
					fread(rombuf, 1, 0x200, usefile_2);
#endif
					PhysPt romseg_pt=host_readw(&rombuf[0x1ce])<<4;

					/* read cartridge data into buffer */
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
					usefile_2->Read_Raw(rombuf, 0x200, rombytesize_2-0x200);
#else
					fseek(usefile_2, 0x200L, SEEK_SET);
					fread(rombuf, 1, rombytesize_2-0x200, usefile_2);
#endif
					//fclose(usefile_2); //usefile_2 is in diskSwap structure which should be deleted to close the file

					/* write cartridge data into ROM */
					for(i=0;i<rombytesize_2-0x200;i++) phys_writeb(romseg_pt+i,rombuf[i]);
				}

#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
				usefile_1->Read_Raw(rombuf, 0x0, 0x200);
#else
				fseek(usefile_1, 0x0L, SEEK_SET);
				fread(rombuf, 1, 0x200, usefile_1);
#endif
				Bit16u romseg=host_readw(&rombuf[0x1ce]);

				/* read cartridge data into buffer */
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
				usefile_1->Read_Raw(rombuf, 0x200, rombytesize_1-0x200);
#else
				fseek(usefile_1,0x200L, SEEK_SET);
				fread(rombuf, 1, rombytesize_1-0x200, usefile_1);
#endif
				//fclose(usefile_1); //usefile_1 is in diskSwap structure which should be deleted to close the file

				/* write cartridge data into ROM */
				for(i=0;i<rombytesize_1-0x200;i++) phys_writeb((romseg<<4)+i,rombuf[i]);

				//Close cardridges
#ifdef C_DBP_ENABLE_DISKSWAP
				for(Bitu dct=0;dct<MAX_SWAPPABLE_DISKS;dct++) {
					if(diskSwap[dct]!=NULL) {
						delete diskSwap[dct];
						diskSwap[dct]=NULL;
					}
				}
#else
				if (usefile_1) delete usefile_1; // might clear imageDiskList[drive-65] if needed
				if (usefile_2) delete usefile_2;
#endif


				if (cart_cmd=="") {
					Bit32u old_int18=mem_readd(0x60);
					/* run cartridge setup */
					SegSet16(ds,romseg);
					SegSet16(es,romseg);
					SegSet16(ss,0x8000);
					reg_esp=0xfffe;
					CALLBACK_RunRealFar(romseg,0x0003);

					Bit32u new_int18=mem_readd(0x60);
					if (old_int18!=new_int18) {
						/* boot cartridge (int18) */
						SegSet16(cs,RealSeg(new_int18));
						reg_ip = RealOff(new_int18);
					} 
				} else {
					if (cfound_at>0) {
						/* run cartridge setup */
						SegSet16(ds,dos.psp());
						SegSet16(es,dos.psp());
						CALLBACK_RunRealFar(romseg,cfound_at);
					}
				}
			}
		} else {
			disable_umb_ems_xms();
			void RemoveEMSPageFrame(void);
			RemoveEMSPageFrame();
			WriteOut(MSG_Get("PROGRAM_BOOT_BOOT"), drive);
			for(i=0;i<512;i++) real_writeb(0, 0x7c00 + i, bootarea.rawdata[i]);

#ifdef C_DBP_ENABLE_IDE
			// Also enable IDE CDROM when using boot from the command line (as opposed to using the Start Menu)
			for (Bit8u i = (Bit8u)('D'-'A'); i != DOS_DRIVES; i++)
			{
				if (!Drives[i] || !dynamic_cast<isoDrive*>(Drives[i])) continue;
				void IDE_SetupControllers(bool alwaysHaveCDROM);
				IDE_SetupControllers(false);
				break;
			}
#endif

			/* create appearance of floppy drive DMA usage (Demon's Forge) */
			if (!IS_TANDY_ARCH && floppysize!=0) GetDMAChannel(2)->tcount=true;

			/* revector some dos-allocated interrupts */
			real_writed(0,0x01*4,0xf000ff53);
			real_writed(0,0x03*4,0xf000ff53);

			SegSet16(cs, 0);
			reg_ip = 0x7c00;
			SegSet16(ds, 0);
			SegSet16(es, 0);
			/* set up stack at a safe place */
			SegSet16(ss, 0x7000);
			reg_esp = 0x100;
			reg_esi = 0;
			reg_ecx = 1;
			reg_ebp = 0;
			reg_eax = 0;
			reg_edx = 0; //Head 0 drive 0
			reg_ebx= 0x7c00; //Real code probably uses bx to load the image
#ifndef C_DBP_ENABLE_DISKSWAP
			if (usefile_1 && imageDiskList[drive-65]!=usefile_1) delete usefile_1; // was already mounted
			if (usefile_2) delete usefile_2;
#endif
		}
	}
};

static void BOOT_ProgramStart(Program * * make) {
	*make=new BOOT;
}


class LOADROM : public Program {
public:
	void Run(void) {
		if (!(cmd->FindCommand(1, temp_line))) {
			WriteOut(MSG_Get("PROGRAM_LOADROM_SPECIFY_FILE"));
			return;
		}

		Bit8u drive;
		char fullname[DOS_PATHLENGTH];
		localDrive* ldp=0;
		if (!DOS_MakeName((char *)temp_line.c_str(),fullname,&drive)) return;

#ifdef C_DBP_ENABLE_EXCEPTIONS //this try catch is meaningless anyway, nothing in it throws
		try {
#endif
			/* try to read ROM file into buffer */
			ldp=dynamic_cast<localDrive*>(Drives[drive]);
			if(!ldp) return;

			FILE *tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
			if(tmpfile == NULL) {
				WriteOut(MSG_Get("PROGRAM_LOADROM_CANT_OPEN"));
				return;
			}
			fseek(tmpfile, 0L, SEEK_END);
			if (ftell(tmpfile)>0x8000) {
				WriteOut(MSG_Get("PROGRAM_LOADROM_TOO_LARGE"));
				fclose(tmpfile);
				return;
			}
			fseek(tmpfile, 0L, SEEK_SET);
			Bit8u rom_buffer[0x8000];
			Bitu data_read = fread(rom_buffer, 1, 0x8000, tmpfile);
			fclose(tmpfile);

			/* try to identify ROM type */
			PhysPt rom_base = 0;
			if (data_read >= 0x4000 && rom_buffer[0] == 0x55 && rom_buffer[1] == 0xaa &&
				(rom_buffer[3] & 0xfc) == 0xe8 && strncmp((char*)(&rom_buffer[0x1e]), "IBM", 3) == 0) {

				if (!IS_EGAVGA_ARCH) {
					WriteOut(MSG_Get("PROGRAM_LOADROM_INCOMPATIBLE"));
					return;
				}
				rom_base = PhysMake(0xc000, 0); // video BIOS
			}
			else if (data_read == 0x8000 && rom_buffer[0] == 0xe9 && rom_buffer[1] == 0x8f &&
				rom_buffer[2] == 0x7e && strncmp((char*)(&rom_buffer[0x4cd4]), "IBM", 3) == 0) {

				rom_base = PhysMake(0xf600, 0); // BASIC
			}

			if (rom_base) {
				/* write buffer into ROM */
				for (Bitu i=0; i<data_read; i++) phys_writeb(rom_base + i, rom_buffer[i]);

				if (rom_base == 0xc0000) {
					/* initialize video BIOS */
					phys_writeb(PhysMake(0xf000, 0xf065), 0xcf);
					reg_flags &= ~FLAG_IF;
					CALLBACK_RunRealFar(0xc000, 0x0003);
					LOG_MSG("Video BIOS ROM loaded and initialized.");
				}
				else WriteOut(MSG_Get("PROGRAM_LOADROM_BASIC_LOADED"));
			}
			else WriteOut(MSG_Get("PROGRAM_LOADROM_UNRECOGNIZED"));
#ifdef C_DBP_ENABLE_EXCEPTIONS
		}
		catch(...) {
			return;
		}
#endif
	}
};

static void LOADROM_ProgramStart(Program * * make) {
	*make=new LOADROM;
}

#if C_DEBUG
class BIOSTEST : public Program {
public:
	void Run(void) {
		if (!(cmd->FindCommand(1, temp_line))) {
			WriteOut("Must specify BIOS file to load.\n");
			return;
		}

		Bit8u drive;
		char fullname[DOS_PATHLENGTH];
		localDrive* ldp = 0;
		if (!DOS_MakeName((char *)temp_line.c_str(), fullname, &drive)) return;

#ifdef C_DBP_ENABLE_EXCEPTIONS //this try catch is meaningless anyway, nothing in it throws
		try {
#endif
			/* try to read ROM file into buffer */
			ldp = dynamic_cast<localDrive*>(Drives[drive]);
			if (!ldp) return;

			FILE *tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
			if (tmpfile == NULL) {
				WriteOut("Can't open a file");
				return;
			}
			fseek(tmpfile, 0L, SEEK_END);
			if (ftell(tmpfile) > 64 * 1024) {
				WriteOut("BIOS File too large");
				fclose(tmpfile);
				return;
			}
			fseek(tmpfile, 0L, SEEK_SET);
			Bit8u buffer[64*1024];
			Bitu data_read = fread(buffer, 1, sizeof( buffer), tmpfile);
			fclose(tmpfile);

			Bit32u rom_base = PhysMake(0xf000, 0); // override regular dosbox bios
			/* write buffer into ROM */
			for (Bitu i = 0; i < data_read; i++) phys_writeb(rom_base + i, buffer[i]);

			//Start executing this bios
			memset(&cpu_regs, 0, sizeof(cpu_regs));
			memset(&Segs, 0, sizeof(Segs));

			
			SegSet16(cs, 0xf000);
			reg_eip = 0xfff0;
		}
#ifdef C_DBP_ENABLE_EXCEPTIONS
		catch (...) {
			return;
		}
#endif
	}
};

static void BIOSTEST_ProgramStart(Program * * make) {
	*make = new BIOSTEST;
}

#endif

// LOADFIX

class LOADFIX : public Program {
public:
	void Run(void);
};

void LOADFIX::Run(void) 
{
	Bit16u commandNr	= 1;
	Bit16u kb			= 64;
	if (cmd->FindCommand(commandNr,temp_line)) {
		if (temp_line[0]=='-') {
			char ch = temp_line[1];
			if ((*upcase(&ch)=='D') || (*upcase(&ch)=='F')) {
				// Deallocate all
				DOS_FreeProcessMemory(0x40);
				WriteOut(MSG_Get("PROGRAM_LOADFIX_DEALLOCALL"),kb);
				return;
			} else {
				// Set mem amount to allocate
				kb = atoi(temp_line.c_str()+1);
				if (kb==0) kb=64;
				commandNr++;
			}
		}
	}
	// Allocate Memory
	Bit16u segment;
	Bit16u blocks = kb*1024/16;
	if (DOS_AllocateMemory(&segment,&blocks)) {
		DOS_MCB mcb((Bit16u)(segment-1));
		mcb.SetPSPSeg(0x40);			// use fake segment
		WriteOut(MSG_Get("PROGRAM_LOADFIX_ALLOC"),kb);
		// Prepare commandline...
		if (cmd->FindCommand(commandNr++,temp_line)) {
			// get Filename
			char filename[128];
			safe_strncpy(filename,temp_line.c_str(),128);
			// Setup commandline
			char args[256+1];
			args[0] = 0;
			bool found = cmd->FindCommand(commandNr++,temp_line);
			while (found) {
				if (strlen(args)+temp_line.length()+1>256) break;
				strcat(args,temp_line.c_str());
				found = cmd->FindCommand(commandNr++,temp_line);
				if (found) strcat(args," ");
			}
			// Use shell to start program
			DOS_Shell shell;
			shell.Execute(filename,args);
			DOS_FreeMemory(segment);		
			WriteOut(MSG_Get("PROGRAM_LOADFIX_DEALLOC"),kb);
		}
	} else {
		WriteOut(MSG_Get("PROGRAM_LOADFIX_ERROR"),kb);	
	}
}

static void LOADFIX_ProgramStart(Program * * make) {
	*make=new LOADFIX;
}

// RESCAN

class RESCAN : public Program {
public:
	void Run(void);
};

void RESCAN::Run(void) 
{
	bool all = false;
	
	Bit8u drive = DOS_GetDefaultDrive();
	
	if(cmd->FindCommand(1,temp_line)) {
		//-A -All /A /All 
		if(temp_line.size() >= 2 && (temp_line[0] == '-' ||temp_line[0] =='/')&& (temp_line[1] == 'a' || temp_line[1] =='A') ) all = true;
		else if(temp_line.size() == 2 && temp_line[1] == ':') {
			lowcase(temp_line);
			drive  = temp_line[0] - 'a';
		}
	}
	// Get current drive
	if (all) {
		for(Bitu i =0; i<DOS_DRIVES;i++) {
			if (Drives[i]) Drives[i]->EmptyCache();
		}
		WriteOut(MSG_Get("PROGRAM_RESCAN_SUCCESS"));
	} else {
		if (drive < DOS_DRIVES && Drives[drive]) {
			Drives[drive]->EmptyCache();
			WriteOut(MSG_Get("PROGRAM_RESCAN_SUCCESS"));
		}
	}
}

static void RESCAN_ProgramStart(Program * * make) {
	*make=new RESCAN;
}

#ifdef C_DBP_ENABLE_INTROPROGRAM
class INTRO : public Program {
public:
	void DisplayMount(void) {
		/* Basic mounting has a version for each operating system.
		 * This is done this way so both messages appear in the language file*/
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_START"));
#if (WIN32)
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_WINDOWS"));
#else			
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_OTHER"));
#endif
		WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_END"));
	}

	void Run(void) {
		/* Only run if called from the first shell (Xcom TFTD runs any intro file in the path) */
		if(DOS_PSP(dos.psp()).GetParent() != DOS_PSP(DOS_PSP(dos.psp()).GetParent()).GetParent()) return;
		if(cmd->FindExist("cdrom",false)) {
			WriteOut(MSG_Get("PROGRAM_INTRO_CDROM"));
			return;
		}
		if(cmd->FindExist("mount",false)) {
			WriteOut("\033[2J");//Clear screen before printing
			DisplayMount();
			return;
		}
		if(cmd->FindExist("special",false)) {
			WriteOut(MSG_Get("PROGRAM_INTRO_SPECIAL"));
			return;
		}
		/* Default action is to show all pages */
		WriteOut(MSG_Get("PROGRAM_INTRO"));
		Bit8u c;Bit16u n=1;
		DOS_ReadFile (STDIN,&c,&n);
		DisplayMount();
		DOS_ReadFile (STDIN,&c,&n);
		WriteOut(MSG_Get("PROGRAM_INTRO_CDROM"));
		DOS_ReadFile (STDIN,&c,&n);
		WriteOut(MSG_Get("PROGRAM_INTRO_SPECIAL"));
	}
};

static void INTRO_ProgramStart(Program * * make) {
	*make=new INTRO;
}
#endif /* C_DBP_ENABLE_INTROPROGRAM */

class IMGMOUNT : public Program {
public:
	void Run(void) {
		//Hack To allow long commandlines
		ChangeToLongCmd();
		/* In secure mode don't allow people to change imgmount points. 
		 * Neither mount nor unmount */
		if(control->SecureMode()) {
			WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
			return;
		}

		char drive;
		std::string label;
		std::vector<std::string> paths;
		std::string umount;
		/* Check for unmounting */
		if (cmd->FindString("-u",umount,false)) {
			WriteOut(UnmountHelper(umount[0]), toupper(umount[0]));
			return;
		}


		std::string type   = "hdd";
		std::string fstype = "fat";
		cmd->FindString("-t",type,true);
		cmd->FindString("-fs",fstype,true);
		if(type == "cdrom") type = "iso"; //Tiny hack for people who like to type -t cdrom

		//Check type and exit early.
		if (type != "floppy" && type != "hdd" && type != "iso") {
			WriteOut(MSG_Get("PROGRAM_IMGMOUNT_TYPE_UNSUPPORTED"),type.c_str());
			return;
		}

		Bit16u sizes[4] = {0};
		bool imgsizedetect = false;
		
		std::string str_size = "";
#ifdef C_DBP_ENABLE_DRIVE_MANAGER
		Bit8u mediaid = 0xF8;

		if (type == "floppy") {
			mediaid = 0xF0;		
		} else if (type == "iso") {
			//str_size="2048,1,65535,0";	// ignored, see drive_iso.cpp (AllocationInfo)
			mediaid = 0xF8;		
			fstype = "iso";
		}
#else
		if (type == "iso") {
			fstype = "iso";
		}
#endif

		cmd->FindString("-size",str_size,true);
		if ((type=="hdd") && (str_size.size()==0)) {
			imgsizedetect = true;
		} else {
			char number[21] = { 0 };const char * scan = str_size.c_str();
			Bitu index = 0;Bitu count = 0;
			/* Parse the str_size string */
			while (*scan && index < 20 && count < 4) {
				if (*scan==',') {
					number[index] = 0;
					sizes[count++] = atoi(number);
					index = 0;
				} else number[index++] = *scan;
				scan++;
			}
			if (count < 4) {
				number[index] = 0; //always goes correct as index is max 20 at this point.
				sizes[count] = atoi(number);
			}
		}

		if(fstype=="fat" || fstype=="iso") {
			// get the drive letter
			if (!cmd->FindCommand(1,temp_line) || (temp_line.size() > 2) || ((temp_line.size()>1) && (temp_line[1]!=':'))) {
				WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_DRIVE"));
				return;
			}
			int i_drive = toupper(temp_line[0]);
			if (!isalpha(i_drive) || (i_drive - 'A') >= DOS_DRIVES || (i_drive - 'A') <0) {
				WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_DRIVE"));
				return;
			}
			drive = static_cast<char>(i_drive);
		} else if (fstype=="none") {
			cmd->FindCommand(1,temp_line);
			if ((temp_line.size() > 1) || (!isdigit(temp_line[0]))) {
				WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY2"));
				return;
			}
			drive = temp_line[0];
			if ((drive<'0') || (drive>=(MAX_DISK_IMAGES+'0'))) {
				WriteOut_NoParsing(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY2"));
				return;
			}
		} else {
			WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FORMAT_UNSUPPORTED"),fstype.c_str());
			return;
		}
		
		// find all file parameters, assuming that all option parameters have been removed
		while(cmd->FindCommand((unsigned int)(paths.size() + 2), temp_line) && temp_line.size()) {
#if defined(C_DBP_SUPPORT_CDROM_MOUNT_DOSFILE) && defined(C_DBP_SUPPORT_DISK_MOUNT_DOSFILE)
			paths.emplace_back();
			DOS_File *test = FindAndOpenDosFile(temp_line.c_str(), NULL, NULL, NULL, &paths.back());
			if (test==NULL) {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FILE_NOT_FOUND"));
				return;
			}
			if (test->IsOpen()) test->Close();
			if (test->RemoveRef() <= 0) delete test;
#else
			
			struct stat test;
			if (stat(temp_line.c_str(),&test)) {
				//See if it works if the ~ are written out
				std::string homedir(temp_line);
				Cross::ResolveHomedir(homedir);
				if(!stat(homedir.c_str(),&test)) {
					temp_line = homedir;
				} else {
					// convert dosbox filename to system filename
					char fullname[CROSS_LEN];
					char tmp[CROSS_LEN];
					safe_strncpy(tmp, temp_line.c_str(), CROSS_LEN);

					Bit8u dummy;
					if (!DOS_MakeName(tmp, fullname, &dummy) || strncmp(Drives[dummy]->GetInfo(),"local directory",15)) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_NON_LOCAL_DRIVE"));
						return;
					}

					localDrive *ldp = dynamic_cast<localDrive*>(Drives[dummy]);
					if (ldp==NULL) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FILE_NOT_FOUND"));
						return;
					}
					ldp->GetSystemFilename(tmp, fullname);
					temp_line = tmp;

					if (stat(temp_line.c_str(),&test)) {
						WriteOut(MSG_Get("PROGRAM_IMGMOUNT_FILE_NOT_FOUND"));
						return;
					}
				}
			}
			if (S_ISDIR(test.st_mode)) {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_MOUNT"));
				return;
			}
			paths.push_back(temp_line);
#endif
		}
		if (paths.size() == 0) {
			WriteOut(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_FILE"));
			return;	
		}
		if (paths.size() == 1)
			temp_line = paths[0];

		if(fstype=="fat") {
			if (imgsizedetect) {
#if defined(C_DBP_SUPPORT_CDROM_MOUNT_DOSFILE) && defined(C_DBP_SUPPORT_DISK_MOUNT_DOSFILE)
				Bit32u disksize;
				DOS_File *diskfile = FindAndOpenDosFile(temp_line.c_str(), &disksize);
				if (!diskfile) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
				Bit32u fcsize = (Bit32u)(disksize / 512L);
				Bit8u buf[512];
				Bit16u readsize = 512;
				if (!diskfile->Read(buf, &readsize)) readsize = 0;
				if (diskfile->IsOpen()) diskfile->Close();
				if (diskfile->RemoveRef() <= 0) delete diskfile;
				if (readsize != 512) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
#else
				FILE * diskfile = fopen_wrap(temp_line.c_str(), "rb+");
				if (!diskfile) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
				fseek(diskfile, 0L, SEEK_END);
				Bit32u fcsize = (Bit32u)(ftell(diskfile) / 512L);
				Bit8u buf[512];
				fseek(diskfile, 0L, SEEK_SET);
				if (fread(buf,sizeof(Bit8u),512,diskfile)<512) {
					fclose(diskfile);
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
					return;
				}
				fclose(diskfile);
#endif
				if ((buf[510]!=0x55) || (buf[511]!=0xaa)) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_GEOMETRY"));
					return;
				}
				Bitu sectors=(Bitu)(fcsize/(16*63));
				if (sectors*16*63!=fcsize) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_GEOMETRY"));
					return;
				}
				sizes[0]=512;	sizes[1]=63;	sizes[2]=16;	sizes[3]=sectors;

				LOG_MSG("autosized image file: %d:%d:%d:%d",sizes[0],sizes[1],sizes[2],sizes[3]);
			}

			if (Drives[drive-'A']) {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_ALREADY_MOUNTED"));
				return;
			}

#ifdef C_DBP_ENABLE_DRIVE_MANAGER
			std::vector<DOS_Drive*> imgDisks;
			std::vector<std::string>::size_type i;
			std::vector<DOS_Drive*>::size_type ct;
			
			for (i = 0; i < paths.size(); i++) {
				DOS_Drive* newDrive = new fatDrive(paths[i].c_str(),sizes[0],sizes[1],sizes[2],sizes[3],0);
				imgDisks.push_back(newDrive);
				if(!(dynamic_cast<fatDrive*>(newDrive))->created_successfully) {
					WriteOut(MSG_Get("PROGRAM_IMGMOUNT_CANT_CREATE"));
					for(ct = 0; ct < imgDisks.size(); ct++) {
						delete imgDisks[ct];
					}
					return;
				}
			}

			// Update DriveManager
			for(ct = 0; ct < imgDisks.size(); ct++) {
				DriveManager::AppendDisk(drive - 'A', imgDisks[ct]);
			}
			DriveManager::InitializeDrive(drive - 'A');

			// Set the correct media byte in the table 
			mem_writeb(Real2Phys(dos.tables.mediaid) + (drive - 'A') * 9, mediaid);
			
			/* Command uses dta so set it to our internal dta */
			RealPt save_dta = dos.dta();
			dos.dta(dos.tables.tempdta);

			for(ct = 0; ct < imgDisks.size(); ct++) {
				DriveManager::CycleDisks(drive - 'A', (ct == (imgDisks.size() - 1)));

				char root[7] = {drive,':','\\','*','.','*',0};
				DOS_FindFirst(root, DOS_ATTR_VOLUME); // force obtaining the label and saving it in label
			}
			dos.dta(save_dta);

			std::string tmp(paths[0]);
			for (i = 1; i < paths.size(); i++) {
				tmp += "; " + paths[i];
			}
			WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"), drive, tmp.c_str());

			DOS_Drive * newdrive = imgDisks[0];
			switch (drive - 'A') {
				case 0:
				case 1:
					if(!((fatDrive *)newdrive)->loadedDisk->hardDrive) {
						if(imageDiskList[drive - 'A'] != NULL) delete imageDiskList[drive - 'A'];
						imageDiskList[drive - 'A'] = ((fatDrive *)newdrive)->loadedDisk;
					}
					break;
				case 2:
				case 3:
					if(((fatDrive *)newdrive)->loadedDisk->hardDrive) {
						if(imageDiskList[drive - 'A'] != NULL) delete imageDiskList[drive - 'A'];
						imageDiskList[drive - 'A'] = ((fatDrive *)newdrive)->loadedDisk;
						updateDPT();
					}
					break;
			}
#else
			void DBP_ImgMountLoadDisks(char drive, const std::vector<std::string>& paths, bool fat, bool iso);
			DBP_ImgMountLoadDisks(drive, paths, true, false);
			
			std::string tmp(paths[0]);
			for (std::vector<std::string>::size_type i = 1; i < paths.size(); i++) {
				tmp += "; " + paths[i];
			}
			WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"), drive, tmp.c_str());
#endif
		} else if (fstype=="iso") {

			if (Drives[drive-'A']) {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_ALREADY_MOUNTED"));
				return;
			}

#ifdef C_DBP_ENABLE_DRIVE_MANAGER
#ifdef C_DBP_NATIVE_CDROM
			MSCDEX_SetCDInterface(0, -1);
#endif
			// create new drives for all images
			std::vector<DOS_Drive*> isoDisks;
			std::vector<std::string>::size_type i;
			std::vector<DOS_Drive*>::size_type ct;
			for (i = 0; i < paths.size(); i++) {
				int error = -1;
				DOS_Drive* newDrive = new isoDrive(drive, paths[i].c_str(), mediaid, error);
				isoDisks.push_back(newDrive);
				switch (error) {
					case 0  :	break;
					case 1  :	WriteOut(MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS"));	break;
					case 2  :	WriteOut(MSG_Get("MSCDEX_ERROR_NOT_SUPPORTED"));	break;
					case 3  :	WriteOut(MSG_Get("MSCDEX_ERROR_OPEN"));				break;
					case 4  :	WriteOut(MSG_Get("MSCDEX_TOO_MANY_DRIVES"));		break;
					case 5  :	WriteOut(MSG_Get("MSCDEX_LIMITED_SUPPORT"));		break;
					case 6  :	WriteOut(MSG_Get("MSCDEX_INVALID_FILEFORMAT"));		break;
					default :	WriteOut(MSG_Get("MSCDEX_UNKNOWN_ERROR"));			break;
				}
				// error: clean up and leave
				if (error) {
					for(ct = 0; ct < isoDisks.size(); ct++) {
						delete isoDisks[ct];
					}
					return;
				}
			}
			// Update DriveManager
			for(ct = 0; ct < isoDisks.size(); ct++) {
				DriveManager::AppendDisk(drive - 'A', isoDisks[ct]);
			}
			DriveManager::InitializeDrive(drive - 'A');
			
			// Set the correct media byte in the table 
			mem_writeb(Real2Phys(dos.tables.mediaid) + (drive - 'A') * 9, mediaid);
#else
			void DBP_ImgMountLoadDisks(char drive, const std::vector<std::string>& paths, bool fat, bool iso);
			DBP_ImgMountLoadDisks(drive, paths, false, true);

			std::vector<std::string>::size_type i;
#endif
			
			// Print status message (success)
			WriteOut(MSG_Get("MSCDEX_SUCCESS"));
			std::string tmp(paths[0]);
			for (i = 1; i < paths.size(); i++) {
				tmp += "; " + paths[i];
			}
			WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"), drive, tmp.c_str());

		} else if (fstype == "none") {
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
			Bit32u imagesize;
			bool writable;
			DOS_File *newDisk = FindAndOpenDosFile(temp_line.c_str(), &imagesize, &writable);
			if (!newDisk) {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
				return;
			}
			imagesize /= 1024;
#else
			FILE *newDisk = fopen_wrap(temp_line.c_str(), "rb+");
			if (!newDisk) {
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_INVALID_IMAGE"));
				return;
			}
			fseek(newDisk,0L, SEEK_END);
			Bit32u imagesize = (ftell(newDisk) / 1024);
#endif
			const bool hdd = (imagesize > 2880);
			//Seems to make sense to require a valid geometry..
			if (hdd && sizes[0] == 0 && sizes[1] == 0 && sizes[2] == 0 && sizes[3] == 0) {
#ifdef C_DBP_SUPPORT_DISK_MOUNT_DOSFILE
				newDisk->Close();
				delete newDisk;
#else
				fclose(newDisk);
#endif
				WriteOut(MSG_Get("PROGRAM_IMGMOUNT_SPECIFY_GEOMETRY"));
				return;
			}

			imageDisk * newImage = new imageDisk(newDisk, temp_line.c_str(), imagesize, hdd);

			if (hdd) newImage->Set_Geometry(sizes[2],sizes[3],sizes[1],sizes[0]);
			if(imageDiskList[drive - '0'] != NULL)
			{
				//DBP: Need to unmount fat drives using this image disk first
				DBP_ASSERT(imageDiskList[drive - '0'] != newImage); // shouldn't be possible with fstype == "none"
				for (Bit16u i=0;i<DOS_DRIVES;i++)
					if (fatDrive* fat_drive = (Drives[i] ? dynamic_cast<fatDrive*>(Drives[i]) : NULL))
						if (fat_drive->loadedDisk == imageDiskList[drive - '0'])
							UnmountHelper('A' + (char)i);
				delete imageDiskList[drive - '0'];
			}
			imageDiskList[drive - '0'] = newImage;
			if ((drive == '2' || drive == '3') && hdd) updateDPT();
			WriteOut(MSG_Get("PROGRAM_IMGMOUNT_MOUNT_NUMBER"),drive - '0',temp_line.c_str());
		}

		// check if volume label is given. be careful for cdrom
		//if (cmd->FindString("-label",label,true)) newdrive->dirCache.SetLabel(label.c_str());
		return;
	}
};

void IMGMOUNT_ProgramStart(Program * * make) {
	*make=new IMGMOUNT;
}


Bitu DOS_SwitchKeyboardLayout(const char* new_layout, Bit32s& tried_cp);
Bitu DOS_LoadKeyboardLayout(const char * layoutname, Bit32s codepage, const char * codepagefile);
const char* DOS_GetLoadedLayout(void);

class KEYB : public Program {
public:
	void Run(void);
};

void KEYB::Run(void) {
	if (cmd->FindCommand(1,temp_line)) {
		if (cmd->FindString("?",temp_line,false)) {
			WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
		} else {
			/* first parameter is layout ID */
			Bitu keyb_error=0;
			std::string cp_string;
			Bit32s tried_cp = -1;
			if (cmd->FindCommand(2,cp_string)) {
				/* second parameter is codepage number */
				tried_cp=atoi(cp_string.c_str());
				char cp_file_name[256];
				if (cmd->FindCommand(3,cp_string)) {
					/* third parameter is codepage file */
					strcpy(cp_file_name, cp_string.c_str());
				} else {
					/* no codepage file specified, use automatic selection */
					strcpy(cp_file_name, "auto");
				}

				keyb_error=DOS_LoadKeyboardLayout(temp_line.c_str(), tried_cp, cp_file_name);
			} else {
				keyb_error=DOS_SwitchKeyboardLayout(temp_line.c_str(), tried_cp);
			}
			switch (keyb_error) {
				case KEYB_NOERROR:
					WriteOut(MSG_Get("PROGRAM_KEYB_NOERROR"),temp_line.c_str(),dos.loaded_codepage);
					break;
				case KEYB_FILENOTFOUND:
					WriteOut(MSG_Get("PROGRAM_KEYB_FILENOTFOUND"),temp_line.c_str());
					WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
					break;
				case KEYB_INVALIDFILE:
					WriteOut(MSG_Get("PROGRAM_KEYB_INVALIDFILE"),temp_line.c_str());
					break;
				case KEYB_LAYOUTNOTFOUND:
					WriteOut(MSG_Get("PROGRAM_KEYB_LAYOUTNOTFOUND"),temp_line.c_str(),tried_cp);
					break;
				case KEYB_INVALIDCPFILE:
					WriteOut(MSG_Get("PROGRAM_KEYB_INVCPFILE"),temp_line.c_str());
					WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
					break;
				default:
					LOG(LOG_DOSMISC,LOG_ERROR)("KEYB:Invalid returncode %x",keyb_error);
					break;
			}
		}
	} else {
		/* no parameter in the command line, just output codepage info and possibly loaded layout ID */
		const char* layout_name = DOS_GetLoadedLayout();
		if (layout_name==NULL) {
			WriteOut(MSG_Get("PROGRAM_KEYB_INFO"),dos.loaded_codepage);
		} else {
			WriteOut(MSG_Get("PROGRAM_KEYB_INFO_LAYOUT"),dos.loaded_codepage,layout_name);
		}
	}
}

static void KEYB_ProgramStart(Program * * make) {
	*make=new KEYB;
}


void DOS_SetupPrograms(void) {
	/*Add Messages */

	MSG_Add("PROGRAM_MOUNT_CDROMS_FOUND","CDROMs encontrados: %d\n");
	MSG_Add("PROGRAM_MOUNT_STATUS_FORMAT","%-5s  %-58s %-12s\n");
	MSG_Add("PROGRAM_MOUNT_STATUS_2","Unidade %c montada como %s\n");
	MSG_Add("PROGRAM_MOUNT_STATUS_1","As unidades atualmente montadas s\x84o:\n");
	MSG_Add("PROGRAM_MOUNT_ERROR_1","A pasta %s n\x84o existe.\n");
	MSG_Add("PROGRAM_MOUNT_ERROR_2","%s n\x84o \x82 uma pasta\n");
	MSG_Add("PROGRAM_MOUNT_ILL_TYPE","Tipo ilegal %s\n");
	MSG_Add("PROGRAM_MOUNT_ALREADY_MOUNTED","Unidade %c j\xa0 montada com %s\n");
	MSG_Add("PROGRAM_MOUNT_USAGE",
		"Uso \033[34;1mMOUNT Letra-da-Unidade Diret\xa2rio-Local\033[0m\n"
		"Por exemplo: MOUNT c %s\n"
		"Isto faz com que o diret\xa2rio %s funcione como a unidade C: dentro do DOSBox.\n"
		"O diret\xa2rio precisa existir.\n");
	MSG_Add("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED","Unidade %c n\x84o montada.\n");
	MSG_Add("PROGRAM_MOUNT_UMOUNT_SUCCESS","A unidade %c foi desmontada com sucesso.\n");
	MSG_Add("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL","As unidades virtuais n\x84o podem ser desmontadas.\n");
	MSG_Add("PROGRAM_MOUNT_WARNING_WIN","\033[31;1mAviso: Montar C:\\ n\x84o \x82 recomendado.\033[0m\n");
	MSG_Add("PROGRAM_MOUNT_WARNING_OTHER","\033[31;1mAviso: Montar / n\x84o \x82 recomendado.\033[0m\n");
#ifdef C_DBP_NATIVE_OVERLAY
	MSG_Add("PROGRAM_MOUNT_OVERLAY_NO_BASE","Por favor, MONTE primeiro uma pasta normal antes de adicionar uma sobreposi\x87\x84o sobre ela.\n");
	MSG_Add("PROGRAM_MOUNT_OVERLAY_INCOMPAT_BASE","A sobreposi\x87\x84o n\x84o \x82 compat\xa1vel com a unidade especificada.\n");
	MSG_Add("PROGRAM_MOUNT_OVERLAY_MIXED_BASE","A sobreposi\x87\x84o precisa ser especificada usando o mesmo endere\x87amento que\na unidade subjacente. Sem misturar caminhos relativos e absolutos.");
	MSG_Add("PROGRAM_MOUNT_OVERLAY_SAME_AS_BASE","A pasta de sobreposi\x87\x84o n\x84o pode ser a mesma que a da unidade subjacente.\n");
	MSG_Add("PROGRAM_MOUNT_OVERLAY_GENERIC_ERROR","Algo deu errado.\n");
	MSG_Add("PROGRAM_MOUNT_OVERLAY_STATUS","Sobreposi\x87\x84o %s na unidade %c montada.\n");
#endif

	MSG_Add("PROGRAM_MEM_CONVEN","%10d Kb de mem\xa2ria convencional livre\n");
	MSG_Add("PROGRAM_MEM_EXTEND","%10d Kb de mem\xa2ria estendida livre\n");
	MSG_Add("PROGRAM_MEM_EXPAND","%10d Kb de mem\xa2ria expandida livre\n");
	MSG_Add("PROGRAM_MEM_UPPER","%10d Kb de mem\xa2ria superior livre em %d blocos (maior UMB %d Kb)\n");

	MSG_Add("PROGRAM_LOADFIX_ALLOC","%d kb alocados.\n");
	MSG_Add("PROGRAM_LOADFIX_DEALLOC","%d kb liberados.\n");
	MSG_Add("PROGRAM_LOADFIX_DEALLOCALL","Mem\xa2ria usada liberada.\n");
	MSG_Add("PROGRAM_LOADFIX_ERROR","Erro de aloca\x87\x84o de mem\xa2ria.\n");

	MSG_Add("MSCDEX_SUCCESS","MSCDEX instalado.\n");
	MSG_Add("MSCDEX_ERROR_MULTIPLE_CDROMS","MSCDEX: Erro: as letras da unidade de v\xa0rios CD-ROM devem ser cont\xa1nuas.\n");
	MSG_Add("MSCDEX_ERROR_NOT_SUPPORTED","MSCDEX: Erro: Ainda n\x84o suportado.\n");
	MSG_Add("MSCDEX_ERROR_PATH","MSCDEX: O local especificado n\x84o \x82 uma unidade de CD-ROM.\n");
	MSG_Add("MSCDEX_ERROR_OPEN","MSCDEX: Erro: Arquivo inv\xa0lido ou n\x84o pode ser aberto.\n");
	MSG_Add("MSCDEX_TOO_MANY_DRIVES","MSCDEX: Erro: Muitas unidades de CD-ROM (m\xa0x: 5). Falha na instala\x87\x84o de MSCDEX.\n");
	MSG_Add("MSCDEX_LIMITED_SUPPORT","MSCDEX: Subpasta montada: suporte limitado.\n");
	MSG_Add("MSCDEX_INVALID_FILEFORMAT","MSCDEX: Erro: O arquivo n\x84o \x82 uma imagem ISO/CUE ou cont\x82m erros.\n");
	MSG_Add("MSCDEX_UNKNOWN_ERROR","MSCDEX: Erro: Erro desconhecido.\n");

	MSG_Add("PROGRAM_RESCAN_SUCCESS","Cache da unidade apagado.\n");

#ifdef C_DBP_ENABLE_INTROPROGRAM
	MSG_Add("PROGRAM_INTRO",
        "\033[32;1mInforma\x87\x84o:\033[0m\n\n"
		"Para informa\x87\x94es b\xa0sicas sobre montagem, digite \033[34;1mintro mount\033[0m\n"
        "Para informa\x87\x94es sobre suporte a CD-ROM, digite \033[34;1mintro cdrom\033[0m\n"
        "Para informa\x87\x94es sobre teclas especiais, digite \033[34;1mintro special\033[0m\n\n"
        "Para obter a \xa3ltima vers\x84o do DOSBox, visite a p\xa0gina da web:\033[34;1m\n"
		"\n"
        "\033[34;1mhttps://www.dosbox.com\033[0m\n"
		"\n"
        "Para mais informa\x87\x94es sobre o DOSBox, visite nossa Wiki:\n"
		"\n"
        "\033[34;1mhttps://www.dosbox.com/wiki\033[0m\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_START",
		"\033[32;1mAqui tem alguns comandos para voc\x88 come\x87ar:\033[0m\n"
		"Antes que voc\x88 possa utilizar os arquivos localizados em seu pr\xa2prio\n"
		"sistema de arquivos, voc\x88 tem que montar a pasta que cont\x82m os arquivos.\n"
		"\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_WINDOWS",
		"\033[44;1m\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n"
		"\xBA \033[32mmount c c:\\jogosdos\\\033[37m cria uma unidade C com o conte\xa3do de c:\\jogosdos.\xBA\n"
		"\xBA                                                                         \xBA\n"
		"\xBA \033[32mc:\\jogosdos\\\033[37m \x82 um exemplo. Substitua por sua pr\xa2pria pasta de jogos.  \033[37m \xBA\n"
		"\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\033[0m\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_OTHER",
		"\033[44;1m\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n"
		"\xBA \033[32mmount c ~/jogosdos\033[37m criar\xa0 uma unidade C com o conte\xa3do de ~/jogosdos.\xBA\n"
		"\xBA                                                                      \xBA\n"
		"\xBA \033[32m~/jogosdos\033[37m \x82 um exemplo. Substitua por sua pr\xa2pria pasta de jogos.\033[37m  \xBA\n"
		"\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
		"\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\033[0m\n"
		);
	MSG_Add("PROGRAM_INTRO_MOUNT_END",
		"Quando mount for completado corretamente voc\x88 pode digitar \033[34;1mc:\033[0m para ir para\n"
		"sua unidade montada C:. Digite \033[34;1mdir\033[0m para mostrar seu conte\xa3do, \033[34;1mcd\033[0m permite\n"
		"que voc\x88 entre em uma pasta (reconhecido por \033[33;1m[]\033[0m em uma pasta listada).\n"
		"Voc\x88 pode executar programas terminados em \033[31m.exe .bat\033[0m e \033[31m.com\033[0m.\n"
		);
	MSG_Add("PROGRAM_INTRO_CDROM",
		"\033[2J\033[32;1mMontando um CD-ROM real ou virtual no DOSBox:\033[0m\n"
		"O DOSBox fornece emula\x87\x84o de CD-ROM em v\xa0rios n\xa1veis.\n"
		"\n"
		"\033[33mN\xa1vel b\xa0sico\033[0m funciona em todos os CD-ROM e pastas normais.\n"
		"Instala o MSCDEX e marca os arquivos como somente leitura.\n"
		"Normalmente, \x82 suficiente para a maioria dos jogos:\n"
		"\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom\033[0m   ou   \033[34;1mmount d C:\\exemplo -t cdrom\033[0m\n"
		"Se n\x84o funcionar, indique ao DOSBox o r\xa2tulo do CD-ROM:\n"
		"\033[34;1mmount d C:\\exemplo -t cdrom -label CDLABEL\033[0m\n"
		"\n"
		"\033[33mN\xa1vel m\x82dio\033[0m adiciona algum suporte de baixo n\xa1vel.\n"
		"Portanto, s\xa2 funciona em unidades de CD-ROM:\n"
		"\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom -usecd \033[33m0\033[0m\n"
		"\n"
		"\033[33mN\xa1vel \xa3ltimo\033[0m o suporte depende do seu Sistema Operacional:\n"
		"Para \033[1mWindows 2000\033[0m, \033[1mWindows XP\033[0m e \033[1mLinux\033[0m:\n"
		"\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom -usecd \033[33m0 \033[34m-ioctl\033[0m\n"
		"Para \033[1mWindows 9x\033[0m com a camada ASPI instalada:\n"
		"\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom -usecd \033[33m0 \033[34m-aspi\033[0m\n"
		"\n"
		"Substitua \033[0;31mD:\\\033[0m pelo local do seu CD-ROM.\n"
		"Substitua o \033[33;1m0\033[0m em \033[34;1m-usecd \033[33m0\033[0m com o n\xa3mero do seu CD-ROM digitando:\n"
		"\033[34;1mmount -cd\033[0m\n"
		);
	MSG_Add("PROGRAM_INTRO_SPECIAL",
		"\033[2J\033[32;1mTeclas especiais:\033[0m\n"
		"Essas s\x84o as teclas de atalho padr\x84o.\n"
		"Elas podem ser alteradas no \033[33mmapeador de controle\033[0m.\n"
		"\n"
		"\033[33;1mALT-ENTER\033[0m   : Ir para tela cheia e voltar.\n"
		"\033[33;1mALT-PAUSE\033[0m   : Pausar o DOSBox.\n"
		"\033[33;1mCTRL-F1\033[0m     : Iniciar o \033[33mmapeador de controle\033[0m.\n"
		"\033[33;1mCTRL-F4\033[0m     : Atualizar o cache de diret\xa2rio para todas as unidades! Trocar imagem de disco montada.\n"
		"\033[33;1mCTRL-ALT-F5\033[0m : Iniciar/Parar a cria\x87\x84o de um filme da tela.\n"
		"\033[33;1mCTRL-F5\033[0m     : Salvar uma captura de tela.\n"
		"\033[33;1mCTRL-F6\033[0m     : Iniciar/Parar a grava\x87\x84o da sa\xa1da de som em um arquivo wave.\n"
		"\033[33;1mCTRL-ALT-F7\033[0m : Iniciar/Parar a grava\x87\x84o de comandos OPL.\n"
		"\033[33;1mCTRL-ALT-F8\033[0m : Iniciar/Parar a grava\x87\x84o de comandos MIDI crus.\n"
		"\033[33;1mCTRL-F7\033[0m     : Reduzir salto de quadro.\n"
		"\033[33;1mCTRL-F8\033[0m     : Aumentar salto de quadro.\n"
		"\033[33;1mCTRL-F9\033[0m     : Encerrar o DOSBox.\n"
		"\033[33;1mCTRL-F10\033[0m    : Capturar/Liberar o mouse.\n"
		"\033[33;1mCTRL-F11\033[0m    : Diminuir a emula\x87\x84o (Diminuir ciclos do emulador).\n"
		"\033[33;1mCTRL-F12\033[0m    : Acelerar a emula\x87\x84o (Aumentar ciclos do emulador).\n"
		"\033[33;1mALT-F12\033[0m     : Desbloquear velocidade (bot\x84o turbo/avan\x87o r\xa0pido).\n"
		);
#endif
    MSG_Add("PROGRAM_BOOT_NOT_EXIST","O arquivo de disco de inicializa\x87\x84o n\x84o existe. Abortando.\n");
    MSG_Add("PROGRAM_BOOT_NOT_OPEN","O arquivo de disco de inicializa\x87\x84o n\x84o pode ser aberto. Abortando.\n");
    MSG_Add("PROGRAM_BOOT_WRITE_PROTECTED","O arquivo de imagem est\xa0 em modo somente leitura! Inicialize em modo protegido.\n");
    MSG_Add("PROGRAM_BOOT_PRINT_ERROR","Este comando inicia o DOSBox de uma imagem de disquete ou de disco r\xa1gido.\n\n"
        "Para este comando, \x82 poss\xa1vel indicar uma sucess\x84o de disquetes intercambi\xa0veis\n"
        "pelo comando de menu, e unidade: especifica de qual unidade montada iniciar.\n"
        "Se n\x84o for especificada nenhuma letra, por padr\x84o ser\xa0 a unidade A. As unidades\n"
        "que podem ser iniciadas s\x84o A, C e D. Para iniciar do disco r\xa1gido (C ou D), a\n"
        "imagem deve estar previamente montada usando o comando \033[34;1mIMGMOUNT\033[0m.\n\n"
        "A sintaxe deste comando \x82 uma das seguintes:\n\n"
        "\033[34;1mBOOT [diskimg1.img diskimg2.img] [-l letra_da_unidade]\033[0m\n\n"
		);
	MSG_Add("PROGRAM_BOOT_UNABLE","N\x84o foi poss\xa1vel inicializar a partir da unidade %c");
	MSG_Add("PROGRAM_BOOT_IMAGE_MOUNTED", "Imagem(ns) de disquete j\xa0 montada(s).\n");
	MSG_Add("PROGRAM_BOOT_IMAGE_OPEN","Abrindo arquivo imagem: %s\n");
	MSG_Add("PROGRAM_BOOT_IMAGE_NOT_OPEN","N\x84o foi poss\xa1vel abrir %s");
	MSG_Add("PROGRAM_BOOT_BOOT","Iniciando a partir da unidade %c...\n");
	MSG_Add("PROGRAM_BOOT_CART_WO_PCJR","Cartucho PCjr encontrado, mas a m\xa0quina n\x84o \x82 um PCjr");
	MSG_Add("PROGRAM_BOOT_CART_LIST_CMDS","Comandos dispon\xa1veis no cartucho PCjr: %s");
	MSG_Add("PROGRAM_BOOT_CART_NO_CMDS","Os comandos do cartucho PCjr n\x84o foram encontrados");

	MSG_Add("PROGRAM_LOADROM_SPECIFY_FILE","Deve especificar o arquivo ROM a ser carregado.\n");
	MSG_Add("PROGRAM_LOADROM_CANT_OPEN","Arquivo ROM n\x84o acess\xa1vel.\n");
	MSG_Add("PROGRAM_LOADROM_TOO_LARGE","Arquivo ROM muito grande.\n");
	MSG_Add("PROGRAM_LOADROM_INCOMPATIBLE","BIOS de v\xa1deo n\x84o suportada por este tipo de m\xa0quina.\n");
	MSG_Add("PROGRAM_LOADROM_UNRECOGNIZED","Arquivo ROM n\x84o reconhecido.\n");
	MSG_Add("PROGRAM_LOADROM_BASIC_LOADED","BASIC ROM carregada.\n");

	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY_DRIVE","\x90 necess\xa0rio especificar a letra da unidade onde a imagem ser\xa0 montada.\n");
	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY2","\x90 necess\xa0rio especificar o n\xa3mero da unidade (0 ou 3) para montar a imagem em\n0,1=fda,fdb;2,3=hda,hdb).\n");
	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY_GEOMETRY",
		"Para imagens de \033[33mCD-ROM\033[0m:   \033[34;1mIMGMOUNT letra-da-unidade caminho-da-imagem -t iso\033[0m\n"
		"\n"
		"Para imagens de \033[33mharddrive\033[0m: \x90 necess\xa0rio especificar a geometria da unidade para discos r\xa1gidos:\n"
		"bytes_por_setor, setores_por_cilindro, cabe\x87as_por_cilindro, quantidade_de_cilindros.\n"
		"\033[34;1mIMGMOUNT letra_da_unidade local_da_imagem -size bps,spc,hpc,cyl\033[0m\n");
	MSG_Add("PROGRAM_IMGMOUNT_INVALID_IMAGE","N\x84o foi poss\xa1vel carregar o arquivo de imagem.\n"
		"Verifique se o caminho est\xa0 correto e a imagem acess\xa1vel.\n");
	MSG_Add("PROGRAM_IMGMOUNT_INVALID_GEOMETRY","N\x84o foi poss\xa1vel obter a geometria de unidade a partir da imagem.\n"
		"Use o par\x83metro -size bps,spc,hpc,cyl para especificar a geometria.\n");
	MSG_Add("PROGRAM_IMGMOUNT_TYPE_UNSUPPORTED","O tipo \"%s\" n\x84o \x82 suportado. Especifique \"hdd\" ou \"floppy\" ou \"iso\".\n");
	MSG_Add("PROGRAM_IMGMOUNT_FORMAT_UNSUPPORTED","O formato \"%s\" n\x84o \x82 suportado. Especifique \"fat\" ou \"iso\" ou \"none\".\n");
	MSG_Add("PROGRAM_IMGMOUNT_SPECIFY_FILE","\x90 preciso especificar o arquivo de imagem para montar.\n");
	MSG_Add("PROGRAM_IMGMOUNT_FILE_NOT_FOUND","Arquivo imagem n\x84o encontrado.\n");
	MSG_Add("PROGRAM_IMGMOUNT_MOUNT","Para montar pastas, use o comando \033[34;1mMOUNT\033[0m e n\x84o o comando \033[34;1mIMGMOUNT\033[0m.\n");
	MSG_Add("PROGRAM_IMGMOUNT_ALREADY_MOUNTED","A unidade j\xa0 est\xa0 montada com essa letra.\n");
	MSG_Add("PROGRAM_IMGMOUNT_CANT_CREATE","N\x84o \x82 poss\xa1vel criar uma unidade a partir de um arquivo.\n");
	MSG_Add("PROGRAM_IMGMOUNT_MOUNT_NUMBER","Unidade n\xa3mero %d est\xa0 montada como %s\n");
	MSG_Add("PROGRAM_IMGMOUNT_NON_LOCAL_DRIVE", "A imagem deve estar em um anfitri\x84o ou em uma unidade local.\n");
	MSG_Add("PROGRAM_IMGMOUNT_MULTIPLE_NON_CUEISO_FILES", "O uso de m\xa3ltiplos arquivos s\xa2 \x82 suportado para imagens cue/iso.\n");

	MSG_Add("PROGRAM_KEYB_INFO","P\xa0gina de c\xa2digo %i carregada\n");
	MSG_Add("PROGRAM_KEYB_INFO_LAYOUT","P\xa0gina de c\xa2digo %i carregada para o esquema %s\n");
	MSG_Add("PROGRAM_KEYB_SHOWHELP","Configura um teclado para um idioma espec\xa1fico.\n\n"
		"Uso: \033[32;1mKEYB\033[0m [ID do esquema do teclado [p\xa0gina de c\xa2digo [arquivo]]]\n"
		"Alguns exemplos:\n"
		"\033[32;1mKEYB\033[0m                 Exibe a p\xa0gina de c\xa2digo carregada atualmente.\n"
		"\033[32;1mKEYB sp\033[0m              Carrega o esquema espanhol (SP) com a\n"
		"                     p\xa0gina de c\xa2digo apropriada.\n"
		"\033[32;1mKEYB sp 850\033[0m          Carrega o esquema espanhol (SP) com a\n"
		"                     p\xa0gina de c\xa2digo 850.\n"
		"\033[32;1mKEYB sp 850 mycp.cpi\033[0m O mesmo de cima, mas usando o arquivo mycp.cpi.\n"
		"\033[32;1mKEYB sp_mod 850\033[0m      Carrega o esquema de sp_mod.kl, usa p\xa0gina de c\xa2digo 850.\n");
	MSG_Add("PROGRAM_KEYB_NOERROR","Esquema de teclado %s carregado para p\xa0gina de c\xa2digo %i\n");
	MSG_Add("PROGRAM_KEYB_FILENOTFOUND","Arquivo de teclado %s n\x84o encontrado\n\n");
	MSG_Add("PROGRAM_KEYB_INVALIDFILE","Arquivo de teclado %s inv\xa0lido\n");
	MSG_Add("PROGRAM_KEYB_LAYOUTNOTFOUND","N\x84o existe esquema em %s para a p\xa0gina de c\xa2digo %i\n");
	MSG_Add("PROGRAM_KEYB_INVCPFILE","Nenhum arquivo de p\xa0gina de c\xa2digo ou \x82 inv\xa0lido para o esquema %s\n\n");

	/*regular setup*/
	PROGRAMS_MakeFile("MOUNT.COM",MOUNT_ProgramStart);
	PROGRAMS_MakeFile("MEM.COM",MEM_ProgramStart);
	PROGRAMS_MakeFile("LOADFIX.COM",LOADFIX_ProgramStart);
	PROGRAMS_MakeFile("RESCAN.COM",RESCAN_ProgramStart);
#ifdef C_DBP_ENABLE_INTROPROGRAM
	PROGRAMS_MakeFile("INTRO.COM",INTRO_ProgramStart);
#endif
	PROGRAMS_MakeFile("BOOT.COM",BOOT_ProgramStart);
#if C_DEBUG
	PROGRAMS_MakeFile("BIOSTEST.COM", BIOSTEST_ProgramStart);
#endif
	PROGRAMS_MakeFile("LOADROM.COM", LOADROM_ProgramStart);
	PROGRAMS_MakeFile("IMGMOUNT.COM", IMGMOUNT_ProgramStart);
	PROGRAMS_MakeFile("KEYB.COM", KEYB_ProgramStart);

}
