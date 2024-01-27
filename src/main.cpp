#include <Geode/Geode.hpp>

#include <regex>

#include <Geode/modify/InfoLayer.hpp>

#include <Geode/modify/LevelCell.hpp>

#include <Geode/modify/CommentCell.hpp>

#include "swears.h"

using namespace geode::prelude;
using namespace std;

// Taken from StackOverflow, original answer by Manuel Martinez
static std::string base64_decode(const std::string & in) {
  std::string out;
  std::vector < int > T(256, -1);
  for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" [i]] = i;

  int val = 0, valb = -8;
  for (unsigned char c: in) {
    if (T[c] == -1) break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

std::string doFilter(std::string message) {
  std::string result = message;
  std::vector < std::string > swears;
  if (Mod::get() -> getSettingValue < bool > ("filter-profanity")) {
    for (const auto & swear: profanity) {
      swears.push_back(swear);
    }
  }
  if (Mod::get() -> getSettingValue < bool > ("filter-sexual")) {
    for (const auto & swear: sexual) {
      swears.push_back(swear);
    }
  }
  if (Mod::get() -> getSettingValue < bool > ("filter-racism")) {
    for (const auto & swear: racism) {
      swears.push_back(swear);
    }
  }
  if (Mod::get() -> getSettingValue < bool > ("filter-antilgbtq")) {
    for (const auto & swear: antilgbtq) {
      swears.push_back(swear);
    }
  }
  for (const auto & currentWord: swears) {
    if (currentWord.empty()) {
      continue;
    }
    std::string replacement = std::string(currentWord.length(), '*');
    if (Mod::get() -> getSettingValue < bool > ("relax-censor")) {
      replacement = currentWord[0] + std::string(currentWord.length() - 2, '*') + currentWord[currentWord.length() - 1];
    }

    std::string extraPattern = "";
    if (Mod::get() -> getSettingValue < bool > ("strict-mode")) {
      extraPattern = "\\b";
    }

    std::regex pattern((std::string) extraPattern + currentWord + (std::string) extraPattern, std::regex_constants::icase);
    result = std::regex_replace(result, pattern, replacement);
  }
  return result;
}

class $modify(CommentCell) {
  void loadFromComment(GJComment * p0) {
    bool warnUnableToFilter = false;
    if (Mod::get() -> getSettingValue < bool > ("censor-comments")) {
      p0 -> m_commentString = doFilter(p0 -> m_commentString);
    }
    CommentCell::loadFromComment(p0);
  }
};

class $modify(InfoLayer) {
  bool init(GJGameLevel * p0, GJUserScore * p1, GJLevelList * p2) {
    if (p0 != NULL) {
      if (Mod::get() -> getSettingValue < bool > ("censor-level-names")) {
        p0 -> m_levelName = doFilter(p0 -> m_levelName.c_str());
      }
      if (Mod::get() -> getSettingValue < bool > ("censor-level-descriptions")) {
        p0 -> m_levelDesc = doFilter(base64_decode(p0 -> m_levelDesc).c_str());
      }
    }
    if (InfoLayer::init(p0, p1, p2) == false) {
      return false;
    }
    return true;
  }
};

class $modify(LevelCell) {
  TodoReturn loadFromLevel(GJGameLevel * p0) {
    if (p0 != NULL) {
      if (Mod::get() -> getSettingValue < bool > ("censor-level-names")) {
        p0 -> m_levelName = doFilter(p0 -> m_levelName.c_str());
      }
      if (Mod::get() -> getSettingValue < bool > ("censor-level-descriptions")) {
        p0 -> m_levelDesc = doFilter(base64_decode(p0 -> m_levelDesc).c_str());
      }
    }
    return LevelCell::loadFromLevel(p0);
  }
};