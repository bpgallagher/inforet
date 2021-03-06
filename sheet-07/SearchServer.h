
// Copyright 2017, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.

#ifndef SEARCHSERVER_H_
#define SEARCHSERVER_H_

#include <boost/asio.hpp>
#include <regex>
#include <locale>
#include <codecvt>
#include "./QGramIndex.h"

// The base directory of the files to serve.
const char SERVE_DIR[] = "./resources/";

// The default HTTP response header.
const char HTTP_OK_HEADER[] = "HTTP/1.1 200 OK";

// The default content type to return in HTTP responses.
const char DEFAULT_CONTENT_TYPE[] = "text/plain";

// The default line delimiter.
const char DEFAULT_LINE_DELIMITER[] = "\r\n";

// The available content types, per file extensions.
const std::map<std::string, std::string> CONTENT_TYPES = {
  { ".html", "text/html" },
  { ".css", "text/css" },
  { ".js", "application/javascript" },
  { ".jpg", "image/jpeg" },
  { ".png", "image/png" },
  { "api", "application/json"}
};

// The available HTTP error responses by status codes.
const std::map<uint16_t, std::string> HTTP_ERROR_RESPONSES = {
  { 403, "HTTP/1.1 403 Forbidden\r\n"
         "Content-type: text/html\r\n"
         "Content-length: 10\r\n\r\n"
         "Forbidden." },
  { 404, "HTTP/1.1 404 Not found\r\n"
         "Content-type: text/html\r\n"
         "Content-length: 10\r\n\r\n"
         "Not found."},
  { 405, "HTTP/1.1 405 Method Not Allowed\r\n"
         "Content-type: text/html\r\n"
         "Content-length: 12\r\n\r\n"
         "Not allowed." }
};

// The regex pattern of the first line from a valid HTTP request header.
const std::regex HTTP_REQUEST_HEADER_REGEX(
    "([A-Z]+) /([a-zA-z0-9.]*)(\\?\\S*)? HTTP/1.1");

// The default encoding.
const char DEFAULT_ENCODING[] = "UTF-8";

// The default file to serve if no fileName is given in the HTTP request.
const char DEFAULT_FILE_NAME[] = "search.html";

// The URL to the default image of an entity, if no image url is given.
const char DEFAULT_ENTITY_IMAGE_URL[] = "noimage.png";

// The path to the HTML pattern file for an entity.
const char ENTITY_TEMPLATE_FILE[] = "entity.pattern.html";

// The number of search results to show per default.
const size_t NUM_SEARCH_RESULTS_TO_SHOW = 5;

// URL, where API is reachable
const char API_URL[] = "api";

// A simple server that handles fuzzy prefix search requests and file requests.
class SearchServer {
 public:
  // Creates a new search server.
  SearchServer(const QGramIndex& index, uint16_t port) :
        _index(index),
        _server(boost::asio::ip::tcp::v4(), port),
        _acceptor(_ioService, _server),
        _client(_ioService),
        _timer(_ioService) {
    // Read the HTML template for the entities of the q-gram index.
    // This is no longer necessary since we serve json
    // std::ifstream ifstream(SERVE_DIR + std::string(ENTITY_TEMPLATE_FILE));
    // std::stringstream buffer;
    // buffer << ifstream.rdbuf();
    // _entityHtmlPattern = buffer.str();
  }

  // Starts the server loop.
  void run();

  // Decode an URL-encoded UTF-8 string
  std::string urlDecode(std::string encoded) const;

 private:
  // Handles a timeout of the client.
  void handleTimeout(const boost::system::error_code& error);

  // Handles a HTTP request.
  void handleRequest(const boost::system::error_code& error);

  // Creates the HTTP response for the given HTTP request.
  std::stringstream createResponse(const std::string& request) const;

  // Handles a fuzzy prefix search request for the query given in 'params', and
  // plugs the result into the given stream (that holds the content of
  // search.html).
  std::stringstream handleFuzzyPrefixSearchRequest(const std::string& params,
      const std::stringstream& stream) const;

  // Translates the given entity to HTML.
  std::string translateToHtml(const Entity& entity) const;

  // Translates the given entity to JSON.
  std::string translateToJSON(const std::vector<Entity>& entities,
      size_t numResultsToShow) const;


  // Returns the content type of the given file.
  std::string getContentType(const std::string& fileName) const;

  // Replaces all occurences of 'from' by 'to' in the given string.
  void replaceAll(std::string& str, const std::string& from,
      const std::string& to) const;

  // The q-gram index to use in this server.
  QGramIndex _index;

  // The server socket.
  boost::asio::ip::tcp::endpoint _server;

  // The service that handles the I/= functionality.
  boost::asio::io_service _ioService;

  // The acceptor.
  boost::asio::ip::tcp::acceptor _acceptor;

  // The client socket.
  boost::asio::ip::tcp::socket _client;

  // The timeout timer.
  boost::asio::deadline_timer _timer;

  // The request buffer.
  boost::asio::streambuf _requestBuffer;

  // The template for the HTML representation of an entity.
  std::string _entityHtmlPattern;
};

#endif  // SEARCHSERVER_H_
