// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "themefileparser.h"

/** ***************************************************************************/
ThemeFileParser::ThemeFileParser(const QString& iniFilePath)
    : iniFile_(iniFilePath, QSettings::IniFormat) {
}


/** ***************************************************************************/
QString ThemeFileParser::path() {
  return iniFile_.fileName();
}


/** ***************************************************************************/
QString ThemeFileParser::name() {
  return iniFile_.value("Icon Theme/Name").toString();
}


/** ***************************************************************************/
QString ThemeFileParser::comment() {
  return iniFile_.value("Icon Theme/Comment").toString();
}


/** ***************************************************************************/
QStringList ThemeFileParser::inherits() {
  QStringList inherits = iniFile_.value("Icon Theme/Inherits").toStringList();
  if ( inherits.isEmpty() && name() != "hicolor" )
      inherits << "hicolor";
  return iniFile_.value("Icon Theme/Inherits").toStringList();
}


/** ***************************************************************************/
QStringList ThemeFileParser::directories() {
  return iniFile_.value("Icon Theme/Directories").toStringList();
}


/** ***************************************************************************/
bool ThemeFileParser::hidden() {
  return iniFile_.value("Icon Theme/Hidden").toBool();
}


/** ***************************************************************************/
int ThemeFileParser::size(const QString& directory) {
  iniFile_.beginGroup(directory);
  int result = iniFile_.value("Size").toInt();
  iniFile_.endGroup();
  return result;
}


/** ***************************************************************************/
QString ThemeFileParser::context(const QString& directory) {
  iniFile_.beginGroup(directory);
  QString result = iniFile_.value("Context").toString();
  iniFile_.endGroup();
  return result;
}


/** ***************************************************************************/
QString ThemeFileParser::type(const QString& directory) {
  iniFile_.beginGroup(directory);
  QString result = iniFile_.contains("Type") ? iniFile_.value("Type").toString()
                                             : "Threshold";
  iniFile_.endGroup();
  return result;
}


/** ***************************************************************************/
int ThemeFileParser::maxSize(const QString& directory) {
  iniFile_.beginGroup(directory);
  int result = iniFile_.contains("MaxSize") ? iniFile_.value("MaxSize").toInt()
                                            : size(directory);
  iniFile_.endGroup();
  return result;
}


/** ***************************************************************************/
int ThemeFileParser::minSize(const QString& directory) {
  iniFile_.beginGroup(directory);
  int result = iniFile_.contains("MinSize") ? iniFile_.value("MinSize").toInt()
                                            : size(directory);
  iniFile_.endGroup();
  return result;
}


/** ***************************************************************************/
int ThemeFileParser::threshold(const QString& directory) {
  iniFile_.beginGroup(directory);
  int result =
      iniFile_.contains("Threshold") ? iniFile_.value("Threshold").toInt() : 2;
  iniFile_.endGroup();
  return result;
}
