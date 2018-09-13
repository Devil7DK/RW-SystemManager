/*
 * Copyright (C) 2018 ATGDroid/Devil7DK
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <errno.h>
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
#include <mntent.h>
#include <selinux/selinux.h>
#include <selinux/label.h>
#include <selinux/android.h>


extern "C"
{
	#include <android/log.h>
}

// Version
#define VERSION "V1.3"

// Tag for Android Log
#define LOG_TAG "RedWolf System Manager"

// Colors for Output
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace std;

void LOGINFO(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	printf(ANSI_COLOR_GREEN);
	vprintf(fmt,ap);
	printf(ANSI_COLOR_RESET);
	__android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
	va_end(ap);
}

void LOGWARN(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	printf(ANSI_COLOR_YELLOW);
	vprintf(fmt,ap);
	printf(ANSI_COLOR_RESET);
	__android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, fmt, ap);
	va_end(ap);
}

void LOGERR(const char* fmt, ...) {
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
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_CYAN<<"                    System Manager "<<VERSION<<"     "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|                                                     |"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"======================================================="<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_MAGENTA<<"     (C) Copyright 2018 ATGDroid/Devil7DK             "<<ANSI_COLOR_BLUE<<"|"<<ANSI_COLOR_RESET<<endl;
	cout<<ANSI_COLOR_BLUE<<"======================================================="<<ANSI_COLOR_RESET<<endl<<endl;
}

struct AppList {
	string App_Name;
	string Pkg_Name;
};

enum environment_variables {
	ANDROID_ROOT,
	ANDROID_DATA,
	EXTERNAL_STORAGE,
	ANDROID_STORAGE
};

struct evn_data_type {
	const char *env_var;
	enum environment_variables env;
	const string default_path;
};

const struct evn_data_type evn_data_types[] = {
	{ "ANDROID_ROOT", ANDROID_ROOT, "/system" },
	{ "ANDROID_DATA", ANDROID_DATA, "/data" },
	{ "EXTERNAL_STORAGE", EXTERNAL_STORAGE, "/sdcard" },
	{ "ANDROID_STORAGE", ANDROID_STORAGE, "/storage" },
	{ NULL, /* break */ },
};

struct piracy_package_data_type {
	const char *app_name;
	const string package_paths;
};

const struct piracy_package_data_type piracy_package_data_types[] = {
	{
		"Lucky Patcher",
			"com.dimonvideo.luckypatcher;" 
			"com.chelpus.lackypatch;"			
			"com.forpda.lp;"
			"com.android.vending.billing.InAppBillingService.LUCK;"
			"com.android.vending.billing.InAppBillingService.CLON;"
			"com.android.vending.billing.InAppBillingService.LOCK;"
			"com.android.vending.billing.InAppBillingService.CRAC;"
			"com.android.vending.billing.InAppBillingService.LACK;"
			"com.android.vending.billing.InAppBillingService.COIN;"
	},
	{
		"Freedom",
			"cc.madkite.freedom;"
	},
	{ 
		"Uret Patcher",
			"zone.jasi2169.uretpatcher;"
			"uret.jasi2169.patcher;"
			"com.android.vendinc;"
			"com.android.vendind;"
	},
	{
		"Pirated Action Launcher 3",
			"p.jasi2169.al3;" 
	},
	{
		"Appcake",
			"com.appcake;"
	},
	{ 
		"ACMarket",
			"ac.market.store;"
	},
	{
		"Game Hacker",
			"org.sbtools.gamehack;"
	},
	{
		"Game Killer",
			"cn.lm.sq;"
			"com.killerapp.gamekiller;"
			"com.zune.gamekiller;"
	},
    {
		"AGK",
			"com.aag.killer;" 
	},
    {
		"CheatIng Hacker",
			"net.schwarzis.game_cih;"
	},
	{
		"Creeplays Hack",
			"org.creeplays.hack;"
	},
	{
		"Base App",
			"com.baseappfull.fwd;"
	},
	{
		"Zm App",
			"com.zmapp;"
	},
	{
		"MarketMod",
			"com.dv.marketmod.installer;"
	},
    {
		"Mobilism",
			"org.mobilism.android;"
	},
    {
		"Blackmart",
			"org.blackmart.market;"
			"com.blackmartalpha;"
	},
	{
		NULL, // break
			""
	},
};


void opendir_failure(const string& path) {
	LOGERR("Error opening directory: '%s' (%s)\n", path.c_str(), strerror(errno));
}

string Get_Environment_Variable(environment_variables var) {
	const struct evn_data_type *environment = evn_data_types;
	
	while (environment->env_var) {
		if (environment->env == var) {
			char const* env = getenv(environment->env_var);
			if (!env) {
				if (!DirExists(environment->default_path)) {
					LOGERR("Failed to find environment variable path for '%s'\n", environment->env_var);
					exit(1);
				}
				return string(environment->default_path);
			} else {
				return string(env); 
			}
		}
		environment++;
	} 
	// Shouldn't ever happen...
	LOGERR("Unknown environment variable requested!\nFailure!\n");
	exit(0);
}

void restorecon(const string entry, const struct stat *sb) {
	char *current_context, *new_context;
	
	if (lgetfilecon(entry.c_str(), &current_context) < 0)
		LOGINFO("Couldn't get selinux context for %s\n", entry.c_str());
	
	if (selabel_lookup(sehandle, &new_context, entry.c_str(), sb->st_mode) < 0)
		LOGINFO("Couldn't lookup selinux context for %s\n", entry.c_str());
	
	if (strcmp(current_context, new_context) != 0) {
		if (lsetfilecon(entry.c_str(), new_context) < 0) {
			LOGINFO("Couldn't label %s with %s: %s\n", entry.c_str(), new_context, strerror(errno));
		}
	}
	freecon(current_context);
	freecon(new_context);
}

void fix_context_recursively(const string path) {
	DIR *d = opendir(path.c_str());
	if (d == NULL) {
		opendir_failure(path);
		return;
	}
	struct dirent *de;
	struct stat sb;
	string local;
	while ((de = readdir(d)) != NULL) {
		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		local = path + "/";
		local.append(de->d_name);
		restorecon(path, &sb);
		if (de->d_type == DT_DIR)
			fix_context_recursively(path);
	}
	closedir(d);
} 

bool Recursive_Mkdir(const string& path) {
	size_t prev_end = 0;
	while (prev_end < path.size()) {
		size_t next_end = path.find('/', prev_end + 1);
		if (next_end == string::npos) {
			break;
		}
		string dir_path = path.substr(0, next_end);
		if (!DirExists(dir_path) && mkdir(dir_path.c_str(), 0777) != 0) {
			LOGERR("Failed to create directory: '%s' (%s)\n", dir_path.c_str(), strerror(errno));
			return false;
		}
		prev_end = next_end;
	}
	return true;
}

int removeDirectory(const string path) {
	DIR *d = opendir(path.c_str());
	int r = 0;
	string new_path;
	
	if (d == NULL) {
		opendir_failure(path);
		return -1;
	}
	
	if (d) {
		struct dirent *p;
		while (!r && (p = readdir(d))) {
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;
			new_path = path + "/";
			new_path.append(p->d_name);
			if (p->d_type == DT_DIR) {
				r = removeDirectory(new_path);
				if (!r) {
					if (p->d_type == DT_DIR)
						r = rmdir(new_path.c_str());
					else
						LOGERR("Unable to remove directory: '%s' (%s)\n", new_path.c_str(), strerror(errno));
				}
			} else if (p->d_type == DT_REG || p->d_type == DT_LNK || p->d_type == DT_FIFO || p->d_type == DT_SOCK) {
				r = unlink(new_path.c_str());
				if (r != 0) {
					LOGERR("Unable to unlink: '%s' (%s)\n", new_path.c_str(), strerror(errno));
				}
			}
		}
		closedir(d);
		
		if (!r)
			r = rmdir(path.c_str());
	}
	return r;
}

bool Execute(const string cmd) {
	FILE* exec;
	bool ret = false;
	char buffer[130];
	exec = popen(cmd.c_str(), "r");
	if (!exec) return ret;
	while (!feof(exec)) {
		if (fgets(buffer, 128, exec) != NULL) {
			if (!ret) ret = true;
		}
	}
	pclose(exec);
	return ret;
}

bool Execute(const string cmd, string &result) {
	FILE* exec;
	bool ret = false;
	char buffer[130];
	exec = popen(cmd.c_str(), "r");
	if (!exec) return ret;
	while (!feof(exec)) {
		if (fgets(buffer, 128, exec) != NULL) {
			result += buffer;
			if (!ret) ret = true;
		}
	}
	pclose(exec);
	return ret;
}

void Fix_Permissions(const string path, const uid_t& userid, const gid_t& groupid) {
	if (chown(path.c_str(), userid, groupid) != 0)
		LOGERR("\t - Chown failed on : %s", path.c_str());
	
	DIR *d = opendir(path.c_str());
	if (d == NULL) {
		opendir_failure(path);
		return;
	}
	struct dirent* de;
	string local;
	while ((de = readdir(d)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		local = path + "/";
		local.append(de->d_name);
		if(de->d_type == DT_DIR) {
			Fix_Permissions(local, userid, groupid);
		} else if (de->d_type == DT_REG) {
			if (chown(path.c_str(), userid, groupid) != 0)
				LOGERR("\t - Chown failed on : %s", path.c_str());
		}
	}
	closedir(dir);
}

bool FileExists(const string name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool DirectoryExists(const string name) {
	struct stat buffer;
	return (stat((name + "/.").c_str(), &buffer) == 0);
}

void piracyWarning(const char *app) {
	char msg[256] = "You can't use this feature of redwolf until the pirating application called '";
	strcat(msg,app);
	strcat(msg,"' is uninstalled from your device!\n\n");
	LOGERR(msg);
}

bool Get_Piracy_Status(const string& data, const string& sdcard) {
	const struct piracy_package_data_type *piracy_apps = piracy_package_data_types;
	size_t start_pos, end_pos;
	string path, android_data = data + "/data";
	while (piracy_apps->app_name) {
		start_pos = 0, end_pos = piracy_apps->package_paths.find(";", start_pos);
		while (end_pos != string::npos && start_pos < piracy_apps->package_paths.size()) {
			path = piracy_apps->package_paths.substr(start_pos, end_pos - start_pos);
			if (DirectoryExists(android_data + "/" + path) || DirectoryExists(sdcard + "/Android/data/" + path)) {
				piracyWarning(piracy_apps->app_name);
				return true;
			}
			start_pos = end_pos + 1;
			end_pos = search.find(";", start_pos);
		}
	}
	if (!DirectoryExists(android_data)) return;
	string folder_path;
	DIR* d;
	struct dirent* de;
	d = opendir(android_data.c_str());
	if (d == NULL) return;
	while ((de = readdir(d)) != NULL) {
		if (de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			folder_path = android_data + "/";
			folder_path.append(de->d_name);
			if (FileExists(folder_path + "/lp/lp_utils"))
				goto app_found;
		}
	}
	if (FileExists(data + "/lp/lp_utils")) goto app_found;
	closedir (d);
	return false;
	app_found:
	closedir (d);
	piracyWarning("Lucky Patcher");
	return true;
}

string remove_trailing_slashes(string s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(),
							   not1(ptr_fun<int, int>(isspace))));
	s.erase(find_if(s.rbegin(), s.rend(),
					not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

bool isInstalled(const string PackageName) {
	string out;
	if (!Execute("pm list packages \"" + PackageName + "\"", out)) {
		return false;
	} else {
		size_t position = out.find_first_of(":");
		if (position == string::npos) {
			LOGINFO("Failed to find divider for installation check: '%s'\n", out.c_str());
			continue;
		}
		if (strcmp(remove_trailing_slashes(out.substr(pos + 1)).c_str(), remove_trailing_slashes(PackageName).c_str()) == 0)
			return true;
		else
			cout<<"return false 2"<<endl;
		return false;
	}
}

void restore_app_data(const string& env, const string PackageName, const string& restore_tar_file) {
	LOGINFO("\t - Restoring Data\n");
	string app_data = env + "/data/" + PackageName;
	
	if (Execute("tar -tf '" + restore_tar_file + "' '" + app_data + "'")) {
		struct stat st;
		bool data_exists = (stat((app_data + "/.").c_str(), &st) == 0);
		uid_t userid;
		gid_t groupid;
		
		if (data_exists) {
			userid = st.st_uid;
			groupid = st.st_gid;
			removeDirectory(app_data);
		}
		
		if (Execute("tar -C / -xf '" + restore_tar_file + "' '" + app_data + "'")){};
		fix_context_recursively(app_data);
		
		if(data_exists)
			Fix_Permissions(app_data, userid, groupid);
	}
}

int main(int argc, char** argv) {
	printBanner();
	
	LOGINFO("Starting " LOG_TAG " " VERSION "...\n\n");
	bool no_sleep = false;
	
	if (argc > 1) {
		if(!strcmp(argv[2], "--no-sleep"))
			no_sleep = true;
	}
	
	if (!no_sleep) {
		LOGINFO("Sleeping For 10 Seconds...\n\n");
		sleep(10);
	}
	
	string DATA_PATH = Get_Environment_Variable(ANDROID_DATA);
	string STORAGE_PATH = Get_Environment_Variable(EXTERNAL_STORAGE);
	string WORK_DIR = STORAGE_PATH + "/WOLF/.BACKUPS/APPS/";
	string TMP_DIR = STORAGE_PATH + "/WOLF/tmp";
	if (Get_Piracy_Status(DATA_PATH, STORAGE_PATH)) return 1;
	
	string restore_list_file = WORK_DIR + "restore.info.dat";
	string restore_tar_file = WORK_DIR + "apps.tar.gz";
	
	if (DirectoryExists(TMP_DIR))
		removeDirectory(TMP_DIR);
	
	if (!Recursive_Mkdir(TMP_DIR)) {
		// error message already displayed by Recursive_Mkdir()
		return 1;
	}
	
	std::vector<AppList> App_List;
	
	LOGINFO("Checking for Files...\n\n");
	if (!FileExists(restore_tar_file)) {
		LOGERR("Unable to find App Backup archive! Aborting...\n");
		return 1;
	}
	if (FileExists(restore_list_file)) {
		LOGINFO("Restore List Found. Parsing List...\n");
		
		ifstream in(restore_list_file);
		
		if (!in) {
			LOGERR("Unable to open restore list... Aborting...\n\n");
			return 1;
		}
		
		string line;
		int lc = 0;
		size_t position;
		bool restore_data = false;
		while (getline(in, line)) {
			line = remove_trailing_slashes(line);
			if (!line.empty() && lc != 0) {
				position = line.find_first_of(":");
				if (position == string::npos)
					continue;
				struct AppList app;
				app.App_Name = line.substr(0, pos);
				app.Pkg_Name = line.substr(pos + 1);
				
				LOGINFO("\tApp Found: %s (%s)\n",app.App_Name.c_str(),app.Pkg_Name.c_str());
				
				App_List.push_back(app);
			} else if (!line.empty() && lc == 0) {
				position = line.find_first_of(":");
				if (position == string::npos) {
					continue;
				}
				if (strcmp(remove_trailing_slashes(line.substr(pos + 1)).c_str(),"1") == 0)
					restore_data = true;
			}
			lc++;
		}
		
		in.close();
		
		LOGINFO("Total Number of apps found in restore list : %zu\n\n", App_List.size());
		if (App_List.size() > 0) {
			LOGINFO("Starting Restore...\n");
			
			int appsRestored = 0;
			for (int i = 0; i < App_List.size(); i++) {
				if (isInstalled(App_List.at(i).Pkg_Name)) {
					LOGWARN("\tApp Already Installed: %s (%s)\n", App_List.at(i).App_Name.c_str(), App_List.at(i).Pkg_Name.c_str());
					if (restore_data) {
						restore_app_data(DATA_PATH, App_List.at(i).Pkg_Name, restore_tar_file);
					}
				} else {
					if (Execute("tar -tf '" + restore_tar_file + "' '" + App_List.at(i).App_Name + ".apk'")) {
						LOGINFO("\tInstalling App: %s (%s)\n", App_List.at(i).App_Name.c_str(), App_List.at(i).Pkg_Name.c_str());
						if (Execute("tar -C '" + TMP_DIR + "' -xf '" + restore_tar_file + "' '" + App_List.at(i).App_Name + ".apk'")){};
						string pm_out;
						Execute("pm install '" + TMP_DIR + "/" + App_List.at(i).App_Name + ".apk'", pm_out);
						if (restore_data) {
							restore_app_data(DATA_PATH, App_List.at(i).Pkg_Name, restore_tar_file);
						}
						
						if (strcmp(remove_trailing_slashes(pm_out).c_str(),"Success") == 0) {
							appsRestored++;
						} else {
							LOGERR(pm_out.c_str());
						}
					} else {
						LOGERR("\tApp Not Found in Archive: %s\n",app.App_Name.c_str());
					}
				}
			}
			LOGINFO("%d of %zu Apps Restored.\n\n", appsRestored, App_List.size());
		}
		unlink(restore_list_file.c_str());
		removeDirectory(TMP_DIR);
	} else {
		LOGINFO("Restore List Not Found. Nothing to do.\n\n");
	}
	
	LOGINFO("System Manager finished...\n\n");
	return 0;
}