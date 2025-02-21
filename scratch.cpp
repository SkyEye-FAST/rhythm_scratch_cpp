#include "toml.hpp"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <random>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// 加载配置
auto const config = toml::parse("configuration.toml");

// 常量
auto const &constTable = toml::find(config, "const");
auto const num = toml::find<int>(constTable, "generate_amount");        // 生成曲目数量
auto const guessChances = toml::find<int>(constTable, "guess_chances"); // 最初刮开可用次数
auto const &pathTable = toml::find(config, "path");
auto const dictFolder = toml::find<std::string>(pathTable, "dict_folder");     // 曲库文件夹
auto const outputFolder = toml::find<std::string>(pathTable, "output_folder"); // 输出文件夹

// 路径
std::string const Path = fs::current_path().string();            // 当前绝对路径
std::string const dictFolderPath = Path + "\\" + dictFolder;     // 曲库路径
std::string const outputFolderPath = Path + "\\" + outputFolder; // 输出路径

std::vector<std::string> loadDict(const std::string &filePath) {
    std::vector<std::string> dictLines;
    std::ifstream file(filePath);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            dictLines.push_back(line);
        }
        file.close();
    }

    return dictLines;
}

std::string knownChar(const std::vector<std::string> &name) {
    std::string result = "已开字符：";
    if (!name.empty()) {
        for (size_t i = 0; i < name.size(); ++i) {
            result += name[i];
            if (i != name.size() - 1) {
                result += "、";
            }
        }
    }
    result += "。";
    return result;
}

void outputToFile(const std::vector<std::string> &name, const std::string &file) {
    std::string outputFilePath = outputFolder + "\\" + file;
    std::ofstream outputFile(outputFilePath);

    if (outputFile.is_open()) {
        for (size_t i = 0; i < name.size(); ++i) {
            outputFile << i + 1 << ". " << name[i] << "\n";
        }
        outputFile.close();
    }
}

void outputToTemp(const std::string &known, const std::vector<std::string> &name, const std::string &file) {
    std::string outputFilePath = outputFolder + "\\" + file;
    std::ofstream outputFile(outputFilePath);

    if (outputFile.is_open()) {
        if (!known.empty()) {
            outputFile << known << "\n";
        }

        for (size_t i = 0; i < name.size(); ++i) {
            outputFile << i + 1 << ". " << name[i] << "\n";
        }

        outputFile.close();
    }
}

void outputLoopPrint(const std::vector<std::string> &name) {
    for (size_t i = 0; i < name.size(); ++i) {
        std::cout << i + 1 << ". " << name[i] << std::endl;
    }
}

void outputHelp() {
    std::cout << "可用命令：" << '\n';
    std::cout << "  help | ? - 显示帮助" << '\n';
    std::cout << "  exit | e - 退出" << '\n';
    std::cout << "  version | ver | v - 列出曲库使用音游版本号" << '\n';
    std::cout << "  (heart | h) add [amount] - 增加可用刮开次数" << '\n';
    std::cout << "  (heart | h) remove [amount] - 减少可用刮开次数" << '\n';
    std::cout << "  (open | o) [character] - 刮开指定字符" << '\n';
    std::cout << "  openspace | os - 刮开空格" << '\n';
    std::cout << "  (check | c) [index] - 将某题全部刮开" << '\n';
    std::cout << "  (show | s) - 显示题目" << '\n';
}

std::vector<std::string> sample(const std::vector<std::string> &input, int num_samples) {
    std::vector<std::string> sampled;
    std::random_device randomDevice;
    std::mt19937 gen(randomDevice());
    std::sample(input.begin(), input.end(), std::back_inserter(sampled), num_samples, gen);
    return sampled;
}

std::wstring stringToWideString(const std::string &str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wideStringToString(const std::wstring &wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

int main() {
    // 创建输出文件夹（若不存在）
    if (!fs::exists(outputFolder)) {
        fs::create_directories(outputFolder);
    }

    std::vector<std::string> allDictsFolder;

    for (auto const &entry : fs::directory_iterator(dictFolderPath)) {
        if (fs::exists(dictFolderPath + "\\" + entry.path().filename().string() + "\\" + "dict.toml")) {
            allDictsFolder.push_back(entry.path().filename().string());
        }
    }

    std::vector<std::string> games;
    std::vector<std::string> versions;

    for (auto const &folder : allDictsFolder) {
        std::string dictConfigFile = dictFolderPath + "\\" + folder + "\\dict.toml";
        const auto dictConfig = toml::parse(dictConfigFile);

        const auto name = toml::find<std::string>(dictConfig, "name");
        const auto version = toml::find<std::string>(dictConfig, "version");
        games.push_back(name);
        versions.push_back(version);
    }

    std::cout << "音游猜曲名刮刮乐\n作者：SkyEye_FAST\n\n可用的曲库：" << '\n';

    // 输出曲库列表
    for (int i = 0; i < games.size(); ++i) {
        std::cout << i + 1 << ". " << games[i] << "（游戏版本：" << versions[i] << "）" << '\n';
    }

    // 选择曲库
    std::cout << "请选择曲库编号，以逗号分隔：" << std::endl;
    std::string input;
    std::cout << "\n>> ";
    std::getline(std::cin, input);
    std::vector<int> selected_dicts;
    size_t start = 0;
    size_t end = input.find(',');
    while (end != std::string::npos) {
        std::string index_str = input.substr(start, end - start);
        index_str.erase(0, index_str.find_first_not_of(' '));
        index_str.erase(index_str.find_last_not_of(' ') + 1);
        if (std::isdigit(index_str[0]) != 0) {
            int index = std::stoi(index_str);
            selected_dicts.push_back(index);
        }
        start = end + 1;
        end = input.find(',', start);
    }
    std::string last_index_str = input.substr(start, end);
    last_index_str.erase(0, last_index_str.find_first_not_of(' '));
    last_index_str.erase(last_index_str.find_last_not_of(' ') + 1);
    if (std::isdigit(last_index_str[0]) != 0) {
        int last_index = std::stoi(last_index_str);
        selected_dicts.push_back(last_index);
    }

    if (selected_dicts.empty()) {
        std::cout << "未按格式输入，请重试。" << '\n';
        return 1;
    }

    // 加载曲库
    std::vector<std::string> selectedDictContent;

    for (int index : selected_dicts) {
        if (1 <= index && index <= allDictsFolder.size()) {
            std::string folder = allDictsFolder[index - 1];
            std::string dictConfigFile = dictFolderPath + "\\" + folder + "\\dict.toml";
            const auto configFile = toml::parse(dictConfigFile);
            const auto dicts = toml::find<std::vector<std::string>>(configFile, "dicts");

            for (const std::string &element : dicts) {
                std::string dictFile = dictFolderPath + "\\" + folder + "\\dict\\" + element;
                std::vector<std::string> dictContent = loadDict(dictFile);
                selectedDictContent.insert(selectedDictContent.end(), dictContent.begin(), dictContent.end());
            }
            std::cout << "已加载曲库“" << games[index - 1] << "”。" << '\n';

        } else {
            std::cout << "编号为“" << index << "”的曲库不存在，忽略。" << '\n';
        }
    }

    // 去除重复曲目
    std::sort(selectedDictContent.begin(), selectedDictContent.end());
    selectedDictContent.erase(std::unique(selectedDictContent.begin(), selectedDictContent.end()), selectedDictContent.end());

    int totalSelected = selectedDictContent.size();
    std::cout << "选择曲目总数：" << totalSelected << '\n';

    // 生成答案
    std::vector<std::string> answerList = sample(selectedDictContent, num);
    outputToFile(answerList, "Answer.txt");

    // 生成初始问题
    std::vector<std::string> questionList;
    for (const std::string &element : answerList) {
        std::string question = std::string(element.size(), '*');
        questionList.push_back(question);
    }
    outputToFile(questionList, "Question.txt");
    std::cout << "\n输入“?”来查看帮助。\n\n题目：" << '\n';
    outputLoopPrint(questionList);

    // 刮卡
    int heart = guessChances;                         // 刮开可用次数
    std::vector<std::string> t = questionList;        // 题目暂存
    std::vector<std::string> tt = questionList;       // 题目暂存的暂存
    std::vector<std::string> openedCharList;          // 刮开的字符，字母大小写
    std::vector<std::string> openedCharLowercaseList; // 刮开的字符，字母全部小写
    bool isAlive = true;

    std::string action;
    while (t != answerList) {
        std::string command;
        std::cout << "\n>> ";
        std::getline(std::cin, command);

        // 替换命令别名
        std::map<std::string, std::string> commandAliases = {
            {"heart", "h"},
            {"open", "o"},
            {"check", "c"},
            {"exit", "e"},
            {"show", "s"},
            {"ver", "v"},
            {"version", "v"},
            {"help", "?"},
            {"？", "?"},
            {"openspace", "os"}};

        std::vector<std::string> parts;
        std::string delimiter = " ";
        size_t pos = 0;
        while ((pos = command.find(delimiter)) != std::string::npos) {
            parts.push_back(command.substr(0, pos));
            command.erase(0, pos + delimiter.length());
        }
        parts.push_back(command);
        action = parts[0];

        if (commandAliases.contains(action)) {
            action = commandAliases[action];
        }

        if (heart <= 0) {
            isAlive = false;
        }

        if (action == "?") {
            outputHelp();
        } else if (action == "e") {
            std::exit(0); // 退出程序
        } else if (action == "v") {
            for (int i = 0; i < games.size(); i++) {
                std::cout << "曲库使用的" << games[i] << "版本：" << versions[i] << '\n';
            }
        } else if (action == "o") {
            if (!isAlive) {
                std::cout << "刮开机会已用完。" << std::endl;
                continue;
            }

            if (parts[1].length() != 1) {
                std::cout << "无效的参数，应为单个字符。" << std::endl;
            } else {
                std::string inputChar = parts[1];
                if (inputChar.find_first_of("-$( )*+.[]{{}}?\\^|/") != std::string::npos) {
                    inputChar = "\\" + inputChar; // 转义字符
                }

                if (std::find(openedCharList.begin(), openedCharList.end(), inputChar) != openedCharList.end()) {
                    std::cout << "这个字符已经刮开了！剩余次数：" << heart << "。" << std::endl;
                } else {
                    openedCharLowercaseList.push_back(parts[1]);
                    if (std::regex_match(inputChar, std::regex("^[\u0041-\u005a\u0061-\u007a\u0391-\u03c9\u0400-\u04ff]$"))) {
                        std::wstring lowercaseChar = stringToWideString(inputChar);
                        for (auto &ch : lowercaseChar) {
                            ch = std::towlower(ch);
                        }
                        std::wstring uppercaseChar = stringToWideString(inputChar);
                        for (auto &ch : uppercaseChar) {
                            ch = std::towupper(ch);
                        }
                        openedCharList.push_back(wideStringToString(lowercaseChar));
                        openedCharList.push_back(wideStringToString(uppercaseChar));
                    } else {
                        openedCharList.push_back(inputChar);
                    }

                    heart -= 1;

                    std::cout << "刮开的字符：" << parts[1] << "。\n剩余次数：" << heart << "。" << std::endl;
                    std::string openedChar;
                    for (const std::string &char_element : openedCharList) {
                        openedChar += char_element;
                    }
                    // 使用正则替换刮开字符
                    t.clear();
                    for (const std::string &element : answerList) {
                        std::string new_element = std::regex_replace(element, std::regex("[^" + openedChar + "]"), "*");
                        t.push_back(new_element);
                    }
                    // 将回答正确的全部刮开
                    for (int i = 0; i < num; i++) {
                        if (t[i] == answerList[i]) {
                            t[i] = answerList[i];
                        }
                    }
                    std::string kc = knownChar(openedCharLowercaseList);
                    outputToTemp(kc, t, "Temp.txt");
                    if (!kc.empty()) {
                        std::cout << kc << std::endl;
                    }
                    outputLoopPrint(t);
                    // 更新暂存
                    tt = t;
                    t.clear();
                }
            }
        } else if (action == "os") {
            if (!isAlive) {
                std::cout << "刮开机会已用完。" << std::endl;
                continue;
            }

            if (std::find(openedCharList.begin(), openedCharList.end(), "空格") != openedCharList.end()) {
                std::cout << "这个字符已经刮开了！剩余次数：" << heart << "。" << std::endl;
            } else {
                openedCharList.emplace_back("\\s");
                openedCharLowercaseList.emplace_back("空格");

                heart -= 1;

                std::cout << "刮开的字符：空格。\n剩余次数：" << heart << "。" << std::endl;
                std::string openedChar;
                for (const std::string &char_element : openedCharList) {
                    openedChar += char_element;
                }
                // 使用正则替换刮开字符
                t.clear();
                for (const std::string &element : answerList) {
                    std::string new_element = std::regex_replace(element, std::regex("[^" + openedChar + "]"), "*");
                    t.push_back(new_element);
                }
                // 将回答正确的全部刮开
                for (int i = 0; i < num; i++) {
                    if (t[i] == answerList[i]) {
                        t[i] = answerList[i];
                    }
                }
                std::string kc = knownChar(openedCharLowercaseList);
                outputToTemp(kc, t, "Temp.txt");
                if (!kc.empty()) {
                    std::cout << kc << std::endl;
                }
                outputLoopPrint(t);
                // 更新暂存
                tt = t;
                t.clear();
            }
        } else if (action == "c") {
            if (std::isdigit(parts[1][0]) == 0) {
                std::cout << "无效的参数，应为数字。" << std::endl;
            } else {
                int n = std::stoi(parts[1]);
                if (n > num || n < 1) {
                    std::cout << "题目不存在。" << std::endl;
                } else if (tt[n - 1] != answerList[n - 1]) {
                    std::cout << "编号为“" << n << "”的题目回答正确，全部刮开。" << std::endl;
                    tt[n - 1] = answerList[n - 1];
                    std::string kc = knownChar(openedCharLowercaseList);
                    outputToTemp(kc, t, "Temp.txt");

                    if (!kc.empty()) {
                        std::cout << kc << std::endl;
                    }

                    outputLoopPrint(tt);
                    t = tt;
                } else {
                    std::cout << "编号为“" << n << "”的题目已经回答正确。" << std::endl;
                }
            }
        } else if (action == "h") {
            if (parts.size() == 3) {
                if (parts[1] == "add" || parts[1] == "remove") {
                    int amount = (parts.size() > 2) ? std::stoi(parts[2]) : 1;
                    if (parts[1] == "add") {
                        heart += amount;
                        std::cout << "已增加" << amount << "次可用刮开次数。\n剩余次数：" << heart << "。" << std::endl;
                    } else {
                        heart -= amount;
                        std::cout << "已减少" << amount << "次可用刮开次数。\n剩余次数：" << heart << "。" << std::endl;
                    }
                } else {
                    std::cout << "无效的命令，请重试。" << std::endl;
                }
            } else {
                std::cout << "无效的命令，请重试。" << std::endl;
            }
        } else if (action == "s") {
            if (t.empty()) {
                outputLoopPrint(tt);
            } else {
                outputLoopPrint(t);
            }
        } else {
            std::cout << "无效的命令，请重试。" << std::endl;
        }
    }

    if (action != "e") {
        std::cout << "\n全部题目已回答正确，答案为：" << std::endl;
        outputLoopPrint(answerList);
    }

    return 0;
}
