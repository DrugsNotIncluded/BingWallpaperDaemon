#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/response.h"
#include "cpr/session.h"
#include <bits/types/time_t.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

std::vector<std::string> ConfigKeywords {"command","interval","region","resolution","path"};
struct ConfigData {
	std::string command, interval, region, resolution, path;
} configdata;

namespace fs = std::filesystem;

const std::vector<std::string> resolutions {"1920x1200", "1920x1080", "1366x768", "1280x768", "1280x720", "1024x768"};
const std::vector<std::string> markets {"en-US", "zh-CN", "ja-JP", "en-AU", "en-UK", "de-DE", "fr-FR", "en-NZ", "en-CA", "es-ES", "es-XL", "pt-BR", "pt-PT" };
const std::string base_url {"http://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1&mkt="};
fs::path user_home (std::getenv("HOME"));
fs::path config_dir (".config");
fs::path default_config_path = user_home / config_dir;
nlohmann::json current_image_info;

nlohmann::json get_daily_wallpaper_info();
std::string get_wallpaper_link_from_info();
void read_parse_config(std::string config_path = default_config_path);
bool check_new_image_awailable();
bool download_file(std::string download_url);
std::string exec(const char* cmd);
time_t date_to_epoch(int year, int month, int day);
void daemon();

nlohmann::json get_daily_wallpaper_info() {
	std::string wallpaper_info_url = base_url + configdata.region;
	cpr::Response r = cpr::Get(cpr::Url{wallpaper_info_url});
	nlohmann::json info = nlohmann::json::parse(r.text);
	return info;
}

std::string get_wallpaper_link_from_info() {
	auto base_image_url = current_image_info["urlbase"];
	return base_image_url.dump() + "_" + configdata.resolution;
}

void read_parse_config(std::string config_path) {
	std::ifstream config_file;
	std::string line;
	while (std::getline(config_file, line)) {
		std::istringstream iss(line);
		std::string keyword, value;
		if (!(iss >> keyword >> value)) {break;}
		
		if (keyword == "command") {configdata.command = keyword;}
		if (keyword == "interval") {configdata.interval = keyword;}
		if (keyword == "region") {configdata.region = keyword;}
		if (keyword == "resolution") {configdata.resolution = keyword;}
		if (keyword == "path") {configdata.path = keyword;}
	}
}

bool check_new_image_awailable() {
	auto info = get_daily_wallpaper_info();
	if (info["hsh"] != current_image_info["hsh"]) {
		current_image_info = info;
		return true;
	}
	return false;
}


bool download_file(std::string download_url) {
	auto ofstream = std::ofstream(configdata.path);
	auto session = cpr::Session();
	session.SetUrl(cpr::Url{download_url});
	auto response = session.Download(ofstream);
	if (response.status_code == 200) {return true;}
	else {return false;}
}

std::string exec(char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    auto pipe = popen(cmd, "r");

    if (!pipe) throw std::runtime_error("popen() failed!");

    while (!feof(pipe)) {
        if (fgets(buffer.data(), 128, pipe) != nullptr)
            result += buffer.data();
    }

    auto rc = pclose(pipe);

    if (rc == EXIT_SUCCESS) {

    } else if (rc == EXIT_FAILURE) {

    }
    return result;
}



time_t date_to_epoch(int year, int month, int day) {
	struct tm t = {0};
	t.tm_year = year - 1900;
	t.tm_mon = month;
	t.tm_mday = day;
	return mktime(&t);
}

void sleep_until(time_t time_until) {
	usleep(time(0)-time_until);
}

void sleep_until_enddate() {
	std::string enddate = current_image_info["enddate"].dump();
	int year = stoi(enddate.substr(0,4));
	int month = stoi(enddate.substr(5,6));
	int day = stoi(enddate.substr(7,8));
	auto enddate_time = date_to_epoch(year, month, day);
	sleep_until(enddate_time);
}

void daemon() {
	// before running daemon "current_image_info" must be init for the first time after startup
	/* get current !!date!! -> if current date is less then image expire data sleep for that time, else check if image hash changed once in configdata.interval, and if it has changed
	   download new image and run wallpaper-manager command, repeat from beginning */

	while (1) {
		// sleep until next day
		sleep_until_enddate();
		// periodically check if new image is awailable, sleep for interval if not
		while (!check_new_image_awailable()) {usleep(stoi(configdata.interval));}
		std::string image_url = get_wallpaper_link_from_info();
		download_file(image_url);
		exec(configdata.command.c_str());
	}
}

void init() {
	read_parse_config();
	current_image_info = get_daily_wallpaper_info();
}

int main(int argc, char *argv[]) {
	init();
	daemon();
	return(0);
}
