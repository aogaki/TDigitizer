#ifndef TParHandler_hpp
#define TParHandler_hpp 1

#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <optional>
#include <vector>

#include "DigiPar.hpp"

class TParHandler
{
 public:
  TParHandler();
  ~TParHandler();

  void UploadParameters(std::string name = "");
  void DownloadParameters();

  std::vector<DigiPar> GetParameters() { return fParameters; };
  void SetParameters(std::vector<DigiPar> par) { fParameters = par; };

 private:
  std::vector<DigiPar> fParameters;

  mongocxx::pool fPool;

  bsoncxx::document::value GetParameterBson(std::string name = "");

  bool GetBool(bsoncxx::document::element &ele, std::string eleName,
               int32_t ch = -1);
  int32_t GetInt32(bsoncxx::document::element &ele, std::string eleName,
                   int32_t ch = -1);
  double GetDouble(bsoncxx::document::element &ele, std::string eleName,
                   int32_t ch = -1);
};

#endif
