#pragma once
#include <QDebug> // Temporary for easy debugging.
#include <QString>

const QString VERSION = "1.0.3";
const QString DISTRIBUTION = "test";
const QString PLATFORM = "win32";
const QString DISTRIBUTION_TOKEN = "test/win32";
const QString USER_AGENT = "TTI-Launcher/1.0.3 (test/win32)";

const QString URL_REPORT_A_BUG = "https://launchpad.net/toontowninfinite";
const QString URL_HOME_PAGE = "https://toontowninfinite.com";
const QString URL_DOWNLOAD_SERVER = "http://download.toontowninfinite.com";
const QString URL_LOGIN_ENDPOINT = "https://toontowninfinite.com/api/login/";

const int ERROR_CODE_NO_CONNECTION = 900;
const int ERROR_CODE_INVALID_RESPONSE = 901;
