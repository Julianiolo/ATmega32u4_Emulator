#include "elfReader.h"

#include <cstring>
#include <algorithm>

#include "StringUtils.h"
#include "DataUtils.h"

uint8_t A32u4::ELF::ELFFile::SymbolTableEntry::getInfoBinding() const {
	return info >> 4;
}
uint8_t A32u4::ELF::ELFFile::SymbolTableEntry::getInfoType() const {
	return info & 0xf;
}

bool A32u4::ELF::ELFFile::DWARF::_debug_line::File::operator==(const File& f) {
	return name == f.name && dir == f.dir && time == f.time && size == f.size;
}

A32u4::ELF::ELFFile::DWARF::_debug_line::CU::Header A32u4::ELF::ELFFile::DWARF::_debug_line::parseCUHeader(const uint8_t* data, size_t dataLen, const ELFHeader::Ident& ident) {
	CU::Header header;
	const uint8_t* ptr = data;

	bool lsb = ident.dataEncoding == ELFFile::ELFHeader::Ident::DataEncoding_LSB;

	header.length                 = (uint32_t)intFromByteArrAdv(&ptr, 4, lsb);
	header.version                = (uint16_t)intFromByteArrAdv(&ptr, 2, lsb);
	header.header_length          = (uint32_t)intFromByteArrAdv(&ptr, 4, lsb);
	header.min_instruction_length = *ptr++;
	header.default_is_stmt        = *ptr++;
	header.line_base              = *ptr++;
	header.line_range             = *ptr++;
	header.opcode_base            = *ptr++;
	std::memcpy(header.std_opcode_lengths, ptr, 9);

	return header;
}
std::vector<A32u4::ELF::ELFFile::DWARF::_debug_line::CU::Entry> A32u4::ELF::ELFFile::DWARF::_debug_line::parseLineByteCode(const uint8_t* data, size_t* off_, size_t end, CU* cu, _debug_line* dl, bool lsb) {
	size_t& off = *off_;
	std::vector<CU::Entry> entrys;
	
	// state machine vars
	uint64_t address = 0;
	uint32_t file = 1;
	uint32_t line = 1;
	uint32_t column = 0; // starts at 1
	bool is_stmt = cu->header.default_is_stmt;
	bool basic_block = false;
	bool end_sequence = false;
	

	const uint32_t const_pc_add = 245 / cu->header.line_range;

	while (off < end) {
		uint8_t opcode = data[off++];
		if (opcode < cu->header.opcode_base) {
			switch (opcode) {
				case DW_LNS_extended_op: 
				{
					uint64_t insn_len = getUleb128(data, &off);
					opcode = data[off++];
					switch (opcode) {
						case DW_LNE_end_sequence:
							end_sequence = true;
							entrys.push_back(CU::Entry{ address, file, line-1, column });
							address = 0;
							file = 1;
							line = 1;
							column = 0; // starts at 1
							is_stmt = cu->header.default_is_stmt;
							basic_block = false;
							end_sequence = false;
							break;
						case DW_LNE_set_address:
							address = intFromByteArr(data + off, 4, lsb);
							off += 4;
							break;

						case DW_LNE_define_file:
						{
							File filedef;
							filedef.name = std::string((const char*)data + off);
							off += filedef.name.size() + 1;
							uint32_t dir = (uint32_t)getUleb128(data, &off) - 1;
							filedef.time = (uint32_t)getUleb128(data, &off);
							filedef.size = (uint32_t)getUleb128(data, &off);

							filedef.dir = dir != (uint32_t)-1 ? (uint32_t)cu->dirs[dir] : -1;

							size_t pos = -1;
							for (size_t i = 0; i < dl->files.size(); i++) {
								if (filedef == dl->files[i]) {
									pos = i;
									break;
								}
							}

							if (pos == (size_t)-1) {
								dl->files.push_back(filedef);
								pos = dl->files.size() - 1;
							}

							cu->files.push_back(pos);
							break;
						}
							
						default:
							off += insn_len;
							break;
					}
					break; //???
				}

				case DW_LNS_copy:
				{
					entrys.push_back(CU::Entry{ address, file, line-1, column });
					basic_block = false;
					break;
				}


				case DW_LNS_advance_pc:
				{
					uint64_t amt = getUleb128(data, &off);
					address += amt * cu->header.min_instruction_length;
					break;
				}


				case DW_LNS_advance_line:
				{
					int64_t amt = getSleb128(data, &off);
					line += (int32_t)amt;
					break;
				}

				case DW_LNS_set_file:
					file = (uint32_t)(getUleb128(data, &off) - 1);
					break;

				case DW_LNS_set_column:
					column = (uint32_t)getUleb128(data, &off);
					break;

				case DW_LNS_negate_stmt:
					is_stmt = !is_stmt;
					break;

				case DW_LNS_set_basic_block:
					basic_block = true;
					break;

				case DW_LNS_const_add_pc:
					address += const_pc_add;
					break;

				case DW_LNS_fixed_advance_pc:
				{
					uint16_t amt = (uint16_t)intFromByteArr(data + off, 2, lsb);
					off += 2;
					address += amt;
					break;
				}
			}

		}
		else {
			// special opcodes
			int adj = opcode - cu->header.opcode_base;
			int addr_adv = adj / cu->header.line_range;
			int line_adv = cu->header.line_base + (adj % cu->header.line_range);

			addr_adv &= 0xff;
			line_adv &= 0xff;

			address += addr_adv;
			line += line_adv;
			basic_block = false;

			entrys.push_back(CU::Entry{ address, file, line-1, column });
		}
	}

	return entrys;
}

A32u4::ELF::ELFFile::DWARF::_debug_line::CU::Entry* A32u4::ELF::ELFFile::DWARF::_debug_line::getEntry(size_t ind) {
	const auto& indexes = entrys[ind];
	return &cus[indexes.first].entrys[indexes.second];
}
const A32u4::ELF::ELFFile::DWARF::_debug_line::CU::Entry* A32u4::ELF::ELFFile::DWARF::_debug_line::getEntry(size_t ind) const {
	const auto& indexes = entrys[ind];
	return &cus[indexes.first].entrys[indexes.second];
}

size_t A32u4::ELF::ELFFile::DWARF::_debug_line::getNumEntrys() const {
	return entrys.size();
}

size_t A32u4::ELF::ELFFile::DWARF::_debug_line::getEntryIndByAddr(uint64_t addr) {
	if (entrys.size() == 0)
		return -1;
	
	size_t from = 0;
	size_t to = entrys.size() - 1;
	while (from != to) {
		size_t mid = from + (to-from) / 2;

		CU::Entry* entry = getEntry(mid);

		if (entry->addr == addr) {
			return mid;
		}
		else if (entry->addr > addr) {
			if (mid == to)
				goto fail;
			to = mid;
		}
		else {
			if (mid == from)
				goto fail;
			from = mid;
		}
	}

	if (getEntry(from)->addr == addr)
		return from;

fail:
	return -1;
}

A32u4::ELF::ELFFile::DWARF::_debug_line A32u4::ELF::ELFFile::DWARF::parse_debug_line(const uint8_t* data, size_t dataLen, const ELFHeader::Ident& ident) {
	_debug_line lines;
	lines.couldFind = true;

	bool lsb = ident.dataEncoding == ELFFile::ELFHeader::Ident::DataEncoding_LSB;

	lines.dirs.clear();
	lines.files.clear();

	size_t off = 0;

	while (off < dataLen) {
		size_t begin = off;
		_debug_line::CU cu;
		cu.header = _debug_line::parseCUHeader(data + off, dataLen - off, ident);
		off += _debug_line::CU::Header::byteSize + 1;
		size_t end = begin + cu.header.length + 4;
		size_t prologueEnd = begin + cu.header.header_length + 9;

		if (cu.header.version != 2 || cu.header.opcode_base != 10) { // not valid dwarf2 
			off = end;
			continue;
		}

		cu.dirs.clear();
		while (off < prologueEnd && data[off]) {
			std::string s((const char*)data + off);
			off += s.size() + 1;
			size_t pos = -1;
			for (size_t i = 0; i < lines.dirs.size(); i++) {
				if (lines.dirs[i] == s) {
					pos = i;
					break;
				}
			}
			if (pos == (size_t)-1) {
				lines.dirs.push_back(s);
				pos = lines.dirs.size() - 1;
			}
			cu.dirs.push_back(pos);
		}
		off++;

		
		while (off < prologueEnd && data[off]) {
			_debug_line::File file;
			file.name = std::string((const char*)data + off);
			off += file.name.size() + 1;

			uint32_t dir = (uint32_t)getUleb128(data, &off) - 1; // index
			file.time    = (uint32_t)getUleb128(data, &off);
			file.size    = (uint32_t)getUleb128(data, &off);

			file.dir = (uint32_t)cu.dirs[dir]; // convert to global merged dir index

			size_t pos = -1;

			for (size_t i = 0; i < lines.files.size(); i++) {
				if (file == lines.files[i]) {
					pos = i;
					break;
				}
			}

			if(pos == (size_t)-1){
				lines.files.push_back(file);
				pos = lines.files.size() - 1;
			}
			cu.files.push_back(pos);
		}
		off++;

		cu.section = {prologueEnd+1, end-off};

		cu.entrys = _debug_line::parseLineByteCode(data, &off, end, &cu, &lines, lsb);
		
		// add Entrys to global Entrys
		for (size_t i = 0; i < cu.entrys.size(); i++) {
			lines.entrys.push_back({ lines.cus.size(), i });
		}

		off = end;

		lines.cus.push_back(cu);
	}

	std::sort(lines.entrys.begin(), lines.entrys.end(), [&](const std::pair<uint32_t, size_t>& a, const std::pair<uint32_t, size_t>& b) {
		return lines.cus[a.first].entrys[a.second].addr < lines.cus[b.first].entrys[b.second].addr;
	});

	for (size_t i = 0; i < lines.files.size(); i++) {
		_debug_line::File& file = lines.files[i];
		std::string path = lines.dirs[file.dir] + "/" + file.name;
		file.content = StringUtils::loadFileIntoString(path.c_str(), &file.couldFind);
		if(file.couldFind)
			file.lines = StringUtils::generateLineIndexArr(file.content.c_str());
	}

	return lines;
}

uint64_t A32u4::ELF::ELFFile::DWARF::getUleb128(const uint8_t* data, size_t* off) {
	uint64_t val = 0;
	uint8_t shift = 0;

	while (true) {
		uint8_t b = data[(*off)++];
		val |= (uint64_t)(b & 0x7f) << shift;
		if ((b & 0x80) == 0)
			break;
		shift += 7;
	}

	return val;
}
int64_t A32u4::ELF::ELFFile::DWARF::getSleb128(const uint8_t* data, size_t* off) {
	int64_t val = 0;
	uint8_t shift = 0;
	uint32_t size = 8 << 3;

	uint8_t b;
	while (true) {
		b = data[(*off)++];
		val |= (b & 0x7f) << shift;
		shift += 7;
		if ((b & 0x80) == 0)
			break;
	}

	if (shift < size && (b & 0x40) != 0)
		val |= -(1 << shift);

	return val;
}

A32u4::ELF::ELFFile::DWARF A32u4::ELF::parseDWARF(const ELFFile& elf) {
	ELFFile::DWARF dwarf;
	size_t debug_line_ind = -1;
	for (size_t i = 0; i < elf.sectionHeaders.size(); i++) {
		if (std::strcmp(elf.shstringTableStr + elf.sectionHeaders[i].name, ".debug_line") == 0) {
			debug_line_ind = i;
			break;
		}
	}

	if (debug_line_ind != (size_t)-1) {
		dwarf.debug_line = ELFFile::DWARF::parse_debug_line(&elf.data[0] + elf.sectionContents[debug_line_ind].first, elf.sectionContents[debug_line_ind].second, elf.header.ident);
	}

	return dwarf;
}


uint64_t A32u4::ELF::intFromByteArr(const uint8_t* data, uint8_t byteLen, bool lsb) {
	uint64_t out = 0;
	if (!lsb) {
		for (size_t i = 0; i < byteLen; i++) {
			out <<= 8;
			out |= data[i];
		}
	}
	else {
		for (size_t i = 0; i < byteLen; i++) {
			out <<= 8;
			out |= data[byteLen - 1 - i];
		}
	}
	return out;
}
uint64_t A32u4::ELF::intFromByteArrAdv(const uint8_t** data, uint8_t byteLen, bool lsb) {
	uint64_t res = intFromByteArr(*data, byteLen, lsb);
	(*data) += byteLen;
	return res;
}

A32u4::ELF::ELFFile::ELFHeader::Ident A32u4::ELF::parseELFHeaderIdentification(const uint8_t* data) {
	ELFFile::ELFHeader::Ident ident;
	const uint8_t* ptr = data;

	for (size_t i = 0; i < 4; i++) {
		ident.magic[i] = *ptr++;
	}
	ident.classtype = *ptr++;
	ident.dataEncoding = *ptr++;
	ident.version = *ptr++;
	ident.OSABI = *ptr++;
	ident.ABIVersion = *ptr++;
	ptr += 7; // padding

	return ident;
}

A32u4::ELF::ELFFile::ELFHeader A32u4::ELF::parseELFHeader(const uint8_t* data, size_t dataLen, size_t* size) {
	ELFFile::ELFHeader header;
	const uint8_t* ptr = data;

	if (dataLen < ELFFile::ELFHeader::Ident::byteSize) {
		abort();
	}

	header.ident = parseELFHeaderIdentification(ptr);
	ptr += ELFFile::ELFHeader::Ident::byteSize;


	bool is64Bit = header.ident.classtype == ELFFile::ELFHeader::Ident::ClassType_64Bit;
	uint8_t addrSize = is64Bit ? 8 : 4;
	size_t headerLenRem = 2 + 2 + 4 + addrSize + addrSize + addrSize + 4 + 2 + 2 + 2 + 2 + 2 + 2;
	if (dataLen < headerLenRem) {
		abort();
	}
	bool lsb = header.ident.dataEncoding == ELFFile::ELFHeader::Ident::DataEncoding_LSB;

	header.type      = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);
	header.machine   = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);

	header.version   = (ELFFile::Word)     intFromByteArrAdv(&ptr, 4, lsb);

	header.entry     = (ELFFile::Word)     intFromByteArrAdv(&ptr, addrSize, lsb);
	header.phoff     = (ELFFile::Word)     intFromByteArrAdv(&ptr, addrSize, lsb);
	header.shoff     = (ELFFile::Word)     intFromByteArrAdv(&ptr, addrSize, lsb);

	header.flags     = (ELFFile::Word)     intFromByteArrAdv(&ptr, 4, lsb);

	header.ehsize    = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);
	header.phentsize = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);
	header.phnum     = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);
	header.shentsize = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);
	header.shnum     = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);
	header.shstrndx  = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);

	if (size)
		*size = ptr - data;

	return header;
}


A32u4::ELF::ELFFile::ProgramHeader A32u4::ELF::parseELFProgramHeader(const uint8_t* data, size_t dataLen, size_t off, const ELFFile::ELFHeader::Ident& ident) {
	ELFFile::ProgramHeader header;
	const uint8_t* ptr = data + off;

	bool lsb = ident.dataEncoding == ELFFile::ELFHeader::Ident::DataEncoding_LSB;
	bool is64Bit = ident.classtype == ELFFile::ELFHeader::Ident::ClassType_64Bit;
	uint8_t addrSize = is64Bit ? 8 : 4;

	header.type      = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);

	if (is64Bit) {
		header.flags = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	}

	header.offset    = (ELFFile::Offset)  intFromByteArrAdv(&ptr, addrSize, lsb);

	header.vaddr     = (ELFFile::Address) intFromByteArrAdv(&ptr, addrSize, lsb);
	header.paddr     = (ELFFile::Address) intFromByteArrAdv(&ptr, addrSize, lsb);

	header.filesz    = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.memsz     = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	if (!is64Bit) {
		header.flags = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	}
	header.align     = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);

	return header;
}

A32u4::ELF::ELFFile::SectionHeader A32u4::ELF::parseELFSectionHeader(const uint8_t* data, size_t dataLen, size_t off, const ELFFile::ELFHeader::Ident& ident) {
	ELFFile::SectionHeader header;
	const uint8_t* ptr = data + off;

	bool lsb = ident.dataEncoding == ELFFile::ELFHeader::Ident::DataEncoding_LSB;
	bool is64Bit = ident.classtype == ELFFile::ELFHeader::Ident::ClassType_64Bit;
	uint8_t addrSize = is64Bit ? 8 : 4;

	header.name      = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.type      = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.flags     = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);

	header.addr      = (ELFFile::Address) intFromByteArrAdv(&ptr, addrSize, lsb);
	header.offset    = (ELFFile::Offset)  intFromByteArrAdv(&ptr, addrSize, lsb);

	header.size      = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.link      = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.info      = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.addralign = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);
	header.entsize   = (ELFFile::Word)    intFromByteArrAdv(&ptr, 4, lsb);

	return header;
}

A32u4::ELF::ELFFile::SymbolTableEntry A32u4::ELF::parseELFSymbol(const uint8_t* data, size_t dataLen, size_t off, const ELFFile::ELFHeader::Ident& ident) {
	ELFFile::SymbolTableEntry symb;
	const uint8_t* ptr = data + off;

	bool lsb = ident.dataEncoding == ELFFile::ELFHeader::Ident::DataEncoding_LSB;
	bool is64Bit = ident.classtype == ELFFile::ELFHeader::Ident::ClassType_64Bit;
	uint8_t addrSize = is64Bit ? 8 : 4;

	symb.name  = (ELFFile::Word)     intFromByteArrAdv(&ptr, 4, lsb);
	symb.value = (ELFFile::Address)  intFromByteArrAdv(&ptr, addrSize, lsb);
	symb.size  = (ELFFile::Word)     intFromByteArrAdv(&ptr, 4, lsb);

	symb.info  = *ptr++;
	symb.other = *ptr++;

	symb.shndx = (ELFFile::HalfWord) intFromByteArrAdv(&ptr, 2, lsb);

	return symb;
}

A32u4::ELF::ELFFile A32u4::ELF::parseELFFile(const uint8_t* data, size_t dataLen) {
	ELFFile file;
	file.data.resize(dataLen);
	memcpy(&file.data[0], data, dataLen);
	const uint8_t* ptr = data;

	size_t headerSize = 0;
	file.header = parseELFHeader(ptr, dataLen, &headerSize);
	ptr += headerSize;

	for (size_t i = 0; i < file.header.phnum; i++) {
		size_t off = file.header.phoff + i * file.header.phentsize;

		ELFFile::ProgramHeader ph = parseELFProgramHeader(data, dataLen, off, file.header.ident);


		file.segmentContents.push_back({ph.offset, ph.filesz});
		file.programHeaders.push_back(ph);
	}

	for (size_t i = 0; i < file.header.shnum; i++) {
		size_t off = file.header.shoff + i * file.header.shentsize;
		
		ELFFile::SectionHeader sh = parseELFSectionHeader(data, dataLen, off, file.header.ident);

		if (sh.type == ELFFile::SectionHeader::Type_SHT_SYMTAB) {
			size_t numEntrys = sh.size / sh.entsize;
			for (size_t i = 0; i < numEntrys; i++) {
				size_t off = sh.offset + i * sh.entsize;
				ELFFile::SymbolTableEntry symb = parseELFSymbol(data, dataLen, off, file.header.ident);
				file.symbolTableEntrys.push_back(symb);
			}
		}
		else if (sh.type == ELFFile::SectionHeader::Type_SHT_STRTAB) {
			if(i == file.header.shstrndx)
				file.shstringTableStr = (const char*)&file.data[0] + sh.offset;
			else
				file.stringTableStr = (const char*)&file.data[0] + sh.offset;
		}

		file.sectionContents.push_back({sh.offset, sh.size});
		file.sectionHeaders.push_back(sh);
	}

	file.dwarf = parseDWARF(file);

	return file;
}

size_t A32u4::ELF::ELFFile::getIndOfSectionWithName(const char* name) const {
	for (size_t i = 0; i < sectionHeaders.size(); i++) {
		if (strcmp(shstringTableStr + sectionHeaders[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

bool A32u4::ELF::ELFFile::hasInfosLoaded() const {
	return data.size() > 0;
}





/*



uint64_t base_address = 0;
uint64_t fileno = 0, lineno = 1;
uint64_t prev_fileno = 0, prev_lineno = 1;
std::string define_file = "";
uint64_t min_address = -1, max_address = 0;




*/