// This is a ROOT macro (or something).
// Read DigiPar.hpp and output body of conversion functions.
// Copy and Pate into somewhere you need.
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &line, std::string delim = " ")
{
  std::vector<std::string> retVec;
  std::string item;
  size_t offset = 0;
  while (true) {
    size_t next = line.find_first_of(delim, offset);
    if (next == std::string::npos) {
      retVec.push_back(line.substr(offset));
      break;
    }
    retVec.push_back(line.substr(offset, next - offset));
    offset = next + 1;
  }

  return retVec;
}

void ParConverter()
{
  const std::string inputFileName = "./DigiPar.hpp";
  auto fin = std::ifstream(inputFileName);

  std::string buf;
  std::vector<std::string> parVec;
  bool pushFlag = false;
  while (!fin.eof()) {
    std::getline(fin, buf);

    auto head = buf.find_first_not_of(" ");
    if (head != std::string::npos) buf.erase(buf.begin(), buf.begin() + head);

    if (buf.find("PAR_DEF_START") != std::string::npos)
      pushFlag = true;
    else if (buf.find("PAR_DEF_STOP") != std::string::npos)
      pushFlag = false;

    if (pushFlag == true && buf.find("//") != 0 && buf.size() > 0)
      parVec.push_back(buf);
  }
  fin.close();

  // Parameters to BSON
  std::cout << "auto builder = bsoncxx::builder::stream::document{};"
            << std::endl;
  for (auto &&line : parVec) {
    auto buf = split(line);
    auto type = buf[0];
    auto varName = buf[1];

    if (varName.find("MAX_CHANNELS") == std::string::npos) {
      std::cout << "builder << \"" + varName;
      if (type != "uint32_t") {
        std::cout << "\" << fParameters." + varName + ";" << std::endl;
      } else {
        std::cout << "\" << static_cast<int32_t>(fParameters." + varName + ");"
                  << std::endl;
      }
    } else {
      auto name = varName.substr(0, varName.find_first_of('['));
      std::cout << "auto arr" + name + " = bsoncxx::builder::stream::array{};"
                << std::endl;
      std::cout << "for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)" << std::endl;
      if (type != "uint32_t") {
        std::cout << "arr" + name + " << fParameters." + name + "[iCh];"
                  << std::endl;
      } else {
        std::cout << "arr" + name + " << static_cast<int32_t>(fParameters." +
                         name + "[iCh]);"
                  << std::endl;
      }
      std::cout << "builder << \"" + name + "\" << arr" + name + ";"
                << std::endl;
    }

    std::cout << "\n";
  }
  std::cout << "bsoncxx::document::value doc = builder << "
               "bsoncxx::builder::stream::finalize;\n"
            << "return doc;" << std::endl;

  std::cout << "\n\n" << std::endl;
  // For BSON to Parameters
  std::cout << "DigiPar par;" << std::endl;
  for (auto &&line : parVec) {
    auto buf = split(line);
    auto type = buf[0];
    auto varName = buf[1];
    // std::cout << line << std::endl;
    if (varName.find("MAX_CHANNELS") == std::string::npos) {
      if (type == "uint32_t" || type == "int") {
        std::cout
            << "par." + varName + " = GetInt32(digitizer, \"" + varName + "\");"
            << std::endl;
      } else if (type == "bool") {
        std::cout
            << "par." + varName + " = GetBool(digitizer, \"" + varName + "\");"
            << std::endl;
      } else if (type == "float" || type == "double") {
        std::cout << "par." + varName + " = GetDouble(digitizer, \"" + varName +
                         "\");"
                  << std::endl;
      } else {
        std::cout << "par." + varName + " = (" + type +
                         ")GetInt32(digitizer, \"" + varName + "\");"
                  << std::endl;
      }
    } else {
      varName = varName.substr(0, varName.find_first_of('['));
      std::cout << "for (auto iCh = 0; iCh < MAX_CHANNELS; iCh++)" << std::endl;
      if (type == "uint32_t" || type == "int") {
        std::cout << "par." + varName + "[iCh] = GetInt32(digitizer, \"" +
                         varName + "\", iCh);"
                  << std::endl;
      } else if (type == "bool") {
        std::cout << "par." + varName + "[iCh] = GetBool(digitizer, \"" +
                         varName + "\", iCh);"
                  << std::endl;
      } else if (type == "float" || type == "double") {
        std::cout << "par." + varName + "[iCh] = GetDouble(digitizer, \"" +
                         varName + "\", iCh);"
                  << std::endl;
      } else {
        std::cout << "par." + varName + "[iCh] = (" + type +
                         ")GetInt32(digitizer, \"" + varName + "\", iCh);"
                  << std::endl;
      }
    }

    std::cout << std::endl;
  }
}
