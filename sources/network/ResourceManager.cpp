#include <ResourceManager.hpp>

ResourceManager::ResourceManager(void) {}

ResourceManager::~ResourceManager(void) { closeAllFileStreams(); }

std::fstream& ResourceManager::openFile(const std::string& path) {
  if (fileStreams.find(path) == fileStreams.end()) {
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>(
        path, std::fstream::in | std::fstream::out);
    if (fs->is_open()) {
      fileStreams.emplace(path, std::move(fs));
    } else {
      throw std::runtime_error("Failed to open file");
    }
  }
  return *fileStreams[path];
}

void ResourceManager::closeFile(const std::string& path) {
  if (fileStreams.find(path) != fileStreams.end()) {
    fileStreams[path]->close();
    fileStreams.erase(path);
  }
}

void ResourceManager::closeAllFileStreams(void) {
  for (auto& fileStream : fileStreams) {
    if (fileStream.second->is_open()) fileStream.second->close();
  }
}