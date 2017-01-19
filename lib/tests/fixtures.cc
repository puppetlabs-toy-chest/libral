#include "fixtures.hpp"
#include <boost/filesystem.hpp>
#include <leatherman/file_util/file.hpp>

temp_directory::temp_directory() {
  auto unique_path = unique_fixture_path();
  dir_name = unique_path.string();

  boost::filesystem::create_directory(unique_path);
}

temp_directory::~temp_directory() {
  boost::filesystem::remove_all(dir_name);
}

std::string const& temp_directory::get_dir_name() const {
  return dir_name;
}

temp_file::temp_file(const std::string &content) {
  auto unique_path = unique_fixture_path();
  file_name = unique_path.string();

  leatherman::file_util::atomic_write_to_file(content, file_name);
}

temp_file::~temp_file() {
  boost::filesystem::remove(file_name);
}

std::string const& temp_file::get_file_name() const {
  return file_name;
}

boost::filesystem::path unique_fixture_path() {
  return boost::filesystem::unique_path("file_util_fixture_%%%%-%%%%-%%%%-%%%%");
}
