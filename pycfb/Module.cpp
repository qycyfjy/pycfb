#include <string>
#include <fstream>
#include <unordered_map>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#define NOMINMAX
#include <Windows.h>

#include "CFB.h"

namespace py = pybind11;

std::wstring ConvertUTF8ToWide(const std::string& s)
{
	int count = MultiByteToWideChar(CP_UTF8, 0, s.data(), -1, NULL, 0);
	std::wstring wstr;
	wstr.reserve(count);
	MultiByteToWideChar(CP_UTF8, 0, s.data(), -1, &wstr[0], count);
	return wstr;
}

struct EntryInfo {
	const CFB::DirectoryEntry* entry;
	size_t depth;

	size_t getDepth() const noexcept { return depth; }

};

struct CFBFile
{
	explicit CFBFile(const std::string& filename) : mFilename(filename) {}
	bool open()
	{
		std::ifstream eif(ConvertUTF8ToWide(mFilename), std::ios::binary);
		if (!eif.is_open()) {
			return false;
		}
		mData = std::string{ std::istreambuf_iterator<char>(eif), std::istreambuf_iterator<char>() };
		mFile.read(mData.data(), mData.size());

		std::unordered_map<std::string, EntryInfo> entries;
		mFile.iterateAll([&entries](const CFB::DirectoryEntry* entry, size_t depth)
			{
				std::string name = CFB::internal::convertUTF16ToUTF8(entry->name);
				entries[name] = EntryInfo{ entry, depth };
			});
		mEntries = std::move(entries);
		return true;
	}

	const std::unordered_map<std::string, EntryInfo>& getEntries() const noexcept {
		return mEntries;
	}

	py::bytes get(const std::string& entryName) {
		if (mEntries.find(entryName) == mEntries.end()) {
			throw std::runtime_error("no such entry");
		}
		std::vector<char> data = mFile.readStreamOfEntry(mEntries[entryName].entry);
		return py::bytes(data.data(), data.size());
	}
private:
	std::string mFilename;
	std::string mData;
	CFB::CompoundFile mFile;
	std::unordered_map<std::string, EntryInfo> mEntries;
};

PYBIND11_MODULE(pycfb, m) {
	m.doc() = "add cfb format support for python";

	py::class_<EntryInfo>(m, "CFBEntry")
		.def("getDepth", &EntryInfo::getDepth);

	py::class_<CFBFile>(m, "CFBFile")
		.def(py::init<const std::string&>())
		.def("open", &CFBFile::open)
		.def("entries", &CFBFile::getEntries)
		.def("get", &CFBFile::get);
}
