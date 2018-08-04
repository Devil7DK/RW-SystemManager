/*==========================================================================*
 *                                                                          *
 *                    (C) Copyright 2018 Devil7 Softwares.                  *
 *                                                                          *
 * Licensed under the Apache License, Version 2.0 (the "License");          *
 * you may not use this file except in compliance with the License.         *
 * You may obtain a copy of the License at                                  *
 *                                                                          *
 *                http://www.apache.org/licenses/LICENSE-2.0                *
 *                                                                          *
 * Unless required by applicable law or agreed to in writing, software      *
 * distributed under the License is distributed on an "AS IS" BASIS,        *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 * See the License for the specific language governing permissions and      *
 * limitations under the License.                                           *
 *                                                                          *
 * Contributors :                                                           *
 *     Dineshkumar T                                                        *
 *     ATG Droid                                                            *
 *                                                                          *
 *==========================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>

extern "C"
{
    #include "selinux-inject.h"
	#include <android/log.h>
}

// Version
#define VERSION "v1.0"

// Tag for Android Log
#define LOG_TAG "RedWolf AfterBoot Binary"

// Colors for Output
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace std;

void LogInfo(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	printf(ANSI_COLOR_GREEN);
	vprintf(fmt,ap);
	printf(ANSI_COLOR_RESET);
	__android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
	va_end(ap);
}

void LogWarn(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	printf(ANSI_COLOR_YELLOW);
	vprintf(fmt,ap);
	printf(ANSI_COLOR_RESET);
	__android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, fmt, ap);
	va_end(ap);
}

void LogError(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	printf(ANSI_COLOR_RED);
	vprintf(fmt,ap);
	printf(ANSI_COLOR_RESET);
	__android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, fmt, ap);
	va_end(ap);
}

void printBanner() {
	cout<<ANSI_COLOR_BLUE<<"======================================================="<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RED<<"               RedWolf Recovery Project              "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"======================================================="<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|                                                     |"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_CYAN<<"      AfterBoot Binary for Special Actions. "<<VERSION<<"     "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|                                                     |"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"======================================================="<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_MAGENTA<<"     (C) Copyright 2018 Devil7 Softwares             "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_MAGENTA<<"                        RedWolf Recovery Project     "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_MAGENTA<<"                        ATG Droid                    "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"======================================================="<<ANSI_COLOR_RESET<<endl<<endl;
}

struct AppList {
	std::string App_Name;
	std::string Pkg_Name;
};

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "error";
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

string exec(string cmd) {
    return exec(cmd.c_str());
}

int getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        return 1;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

bool DirectoryExists( const char* pzPath )
{
    if ( pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (pzPath);

    if (pDir != NULL)
    {
        bExists = true;    
        (void) closedir (pDir);
    }

    return bExists;
}

bool FileExists(const string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

void piracyWarning(const char *app) {
	char msg[200] = "You can't use this feature of redwolf until the pirating application called '";
	strcat(msg,app);
	strcat(msg,"' is uninstalled from your device!\n\n");
	LogError(msg);
}

bool isPirate() {
	string piracy_apps[30] = {"cc.madkite.freedom", "zone.jasi2169.uretpatcher",
							"uret.jasi2169.patcher", "p.jasi2169.al3", "com.dimonvideo.luckypatcher",
							"com.chelpus.lackypatch", "com.forpda.lp", "com.android.vending.billing.InAppBillingService.LUCK",
							"com.android.vending.billing.InAppBillingService.CLON", "com.android.vending.billing.InAppBillingService.LOCK",
							"com.android.vending.billing.InAppBillingService.CRAC", "com.android.vending.billing.InAppBillingService.LACK",
							"com.android.vendinc", "com.appcake", "ac.market.store", "org.sbtools.gamehack", "com.zune.gamekiller",
							"com.aag.killer", "com.killerapp.gamekiller", "cn.lm.sq", "net.schwarzis.game_cih", "org.creeplays.hack",
							"com.baseappfull.fwd", "com.zmapp", "com.dv.marketmod.installer", "org.mobilism.android", "com.blackmartalpha",
							"org.blackmart.market", "com.android.vendind", "com.android.vending.billing.InAppBillingService.COIN"};
	string data_dirs[2] = {"/data/data", "/sdcard/Android/data"};
	
	string pirateApp = "";
	
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 30; j++) {
			if (DirectoryExists(((string)data_dirs[i] + "/" + piracy_apps[j]).c_str())) {				
				switch (j) {
					case 0:
						pirateApp = "Freedom";
						break;
					case 1:
					case 2:
					case 12:
					case 28:
						pirateApp = "Uret Patcher";
						break;
					case 3:
						pirateApp = "Pirated Action Launcher 3";
						break;
					case 4 ... 11:
					case 29:
						pirateApp = "Lucky Patcher";
						break;
					case 13:
						pirateApp = "Appcake";
						break;
					case 14:
						pirateApp = "ACMarket";
						break;
					case 15:
						pirateApp = "Game Hacker";
						break;
					case 16:
					case 18:
					case 19:
						pirateApp = "Game Killer";
						break;
					case 17:
						pirateApp = "AGK";
						break;
					case 20:
						pirateApp = "CheatIng Hacker";
						break;
					case 21:
						pirateApp = "Creeplays Hack";
						break;  
					case 22:
						pirateApp = "Base App";
						break;
					case 23:
						pirateApp = "Zm App";
						break;
					case 24:
						pirateApp = "MarketMod";
						break;
					case 25:
						pirateApp = "Mobilism";
						break;
					case 26:
					case 27:
						pirateApp = "Blackmart";
						break;
				}
			}				
		}
	}
	
	if (pirateApp == "") {
		if (exec("ls /data/lp/lp_utils /data/data/*/lp/lp_utils 2> /dev/null") == "") {
			return false;
		} else {
			piracyWarning("Lucky Patcher");
			return true;
		}		
	} else {
		piracyWarning(pirateApp.c_str());
		return true;
	}
}

vector<string> split_string(const string &in, char del, bool skip_empty) {
        vector<string> res;

        if (in.empty() || del == '\0')
                return res;

        string field;
        istringstream f(in);
        if (del == '\n') {
                while (getline(f, field)) {
                        if (field.empty() && skip_empty)
                                continue;
                        res.push_back(field);
                }
        } else {
                while (getline(f, field, del)) {
                        if (field.empty() && skip_empty)
                                continue;
                        res.push_back(field);
                }
        }
        return res;
}

string trim(string s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(),
            not1(ptr_fun<int, int>(isspace))));
	s.erase(find_if(s.rbegin(), s.rend(),
            not1(ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}

void trim(char* str) {
    int start = 0;
    char* buffer = str;
    while (*str && *str++ == ' ') ++start;
    while (*str++);
    int end = str - buffer - 1; 
    while (end > 0 && buffer[end - 1] == ' ') --end;
    buffer[end] = 0;
    if (end <= start || start == 0) return; // exit if no leading spaces or string is now empty
    str = buffer + start;
    while ((*buffer++ = *str++));  // remove leading spaces: K&R
}

bool isInstalled(string PackageName) {
	string out = exec("pm list packages \"" + PackageName + "\" 2>/dev/null");
	if (out.empty()) {
		return false;
	} else {
		string found = split_string(out,':',false)[1];
		if (strcmp(found.c_str(), PackageName.c_str()))
			return true;
		else
			return false;
	}
}

int main() {
	printBanner();
	
	LogInfo("Starting RedWolf AfterBoot Binary " VERSION "...\n\n");
	
	LogInfo("Sleeping For 1 Minute...\n\n");
	sleep(60);
	
	if (!isPirate()) {
		string restore_list_file = "/sdcard/WOLF/.BACKUPS/APPS/restore.info.dat";
		string restore_tar_file = "/sdcard/WOLF/.BACKUPS/APPS/apps.tar.gz";
		string restore_boot_file = "/sdcard/WOLF/.BACKUPS/APPS/boot.img.bak";
		string tmp_dir = "/sdcard/WOLF/tmp";
		
		if (!DirectoryExists(tmp_dir.c_str())) {
			LogInfo("Creating Temp Dir...\n\n");
			string cmd = "mkdir -p " + tmp_dir;
			system(cmd.c_str());
		}
		
		std::vector<AppList> App_List;
		
		LogInfo("Checking for Files...\n\n");
		if (!FileExists(restore_tar_file)) {
			LogError("Unable to Find App Backup Archive... Aborting...!\n");
			return 1;
		}
		if (FileExists(restore_list_file)) {
			LogInfo("Restore List Found. Parsing List...\n");
			
			ifstream in(restore_list_file);
			
			if (!in) {
				LogError("Unable to open restore list... Aborting...\n\n");
				return 1;
			}
			
			string line;
			while (getline(in, line)) {
				string search = trim(line);
				if (search != "" && strchr(search.c_str(),':') != NULL) {
					vector<string> splited = split_string(line,':',false);
					
					struct AppList app;
					app.App_Name = trim(splited[0]);
					app.Pkg_Name = trim(splited[1]);
					
					LogInfo("\tApp Found: %s (%s)\n",splited[0].c_str(),splited[1].c_str());
					
					App_List.push_back(app);
				}
			}
			
			in.close();
			
			LogInfo("Total Number of apps found in restore list : %zu\n\n", App_List.size());
			if (App_List.size() > 0) {
				LogInfo("Injecting SELinux... \n");
				
				cout<<ANSI_COLOR_BLUE;
				int selinux1 = set_live("init", "init", "file", "execute_no_trans"); // Allow Installing from Init
				int selinux2 = set_live("system_server", "sdcardfs", "file", "read"); // Allow Installing from Shell
				cout<<ANSI_COLOR_RESET;
				if (selinux1 == 0 && selinux2 == 0){
					LogInfo("Success.\n\n");
					LogInfo("Starting Restore...\n");
					
					int appsRestored = 0;
					for (int i = 0; i < App_List.size(); i++) {
						struct AppList app = App_List[i];
						if (isInstalled(app.Pkg_Name)) {
							LogWarn("\tApp Already Installed: %s (%s)\n", app.App_Name.c_str(), app.Pkg_Name.c_str());
						} else {
							string cmd = "tar -tf '" + restore_tar_file + "' '" + app.App_Name +".apk' 2>/dev/null";
							
							if(exec(cmd.c_str())!="") {
								LogInfo("\tInstalling App: %s (%s)\n", app.App_Name.c_str(), app.Pkg_Name.c_str());
								exec("tar -C '" + tmp_dir + "' -xf '" + restore_tar_file + "' '" + app.App_Name + ".apk'");
								const char* pm_out = exec("pm install '" + tmp_dir + "/" + app.App_Name + ".apk'").c_str();
								
								if (strcmp(pm_out,"Success")) {
									appsRestored++;
								} else {
									LogError(pm_out);
								}
							} else {
								LogError("\tApp Not Found in Archive: %s\n",app.App_Name.c_str());
							}
						}
					}
					LogInfo("%d of %zu Apps Restored.\n\n", appsRestored, App_List.size());
				} else {
					LogError("Failed.\n\n");
				}
			}
			
			LogInfo("Deleting Retore Info... ");
			if (unlink(restore_list_file.c_str()) == 0)
				LogInfo("Done.\n\n");
			else
				LogError("Failed!\n\n");
			
			LogInfo("Deleting Temp Dir... \n\n");
			string cmd_rmdir = "rm -rf " + tmp_dir;
			system(cmd_rmdir.c_str());
			
			if (FileExists(restore_boot_file)) {
				LogInfo("Boot Partition Backup Found...");
				LogWarn("Flashing Boot Partion... ");
				string dd_cmd = "dd if='" + restore_boot_file + "' of=/dev/block/bootdevice/by-name/boot 2>&1 > /dev/null";
				
				int ret = system(dd_cmd.c_str());
				if (WEXITSTATUS(ret) == 0x10)
					LogInfo("Success.\n\n");
				else
					LogError("Failed!\n\n");
			}
		} else {
			LogInfo("Restore List Not Found. Nothing to do.\n\n");
		}
	} else {
		LogError("Pirating Apps found on device. Aborting...\n\n");
		return 1;
	}

	LogInfo("Finishing Process.\n\n");
	
	return 0;
}