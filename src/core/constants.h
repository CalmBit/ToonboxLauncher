#pragma once
#include <QDebug> // Temporary for easy debugging.
#include <QString>

const QString VERSION = "1.0.3";
const QString DISTRIBUTION = "test";
const QString USER_AGENT = "TTI-Launcher/1.0.3 (test/win32)";
const QString MANIFEST_FILENAME = "patcher.xml";

const QString URL_REPORT_A_BUG = "https://launchpad.net/toontowninfinite";
const QString URL_HOME_PAGE = "https://toontowninfinite.com";
const QString URL_LOGIN_ENDPOINT = "https://toontowninfinite.com/api/login/";
const QString URL_DOWNLOAD_MIRROR = "http://download.toontowninfinite.com/test/win32/";

const int ERROR_CODE_NO_CONNECTION = 900;
const int ERROR_CODE_INVALID_RESPONSE = 901;
const int ERROR_CODE_WRITE = 902;
const int ERROR_CODE_READ = 903;
const int ERROR_CODE_EXTRACT = 904;
