#include "StringUtils.h"

#include <cstring>
#include <ctype.h>

#include <fstream>
#include <streambuf>

char StringUtils::texBuf[128];

void StringUtils::uIntToHexBufCase(uint64_t num, uint8_t digits, char* buf, bool upperCase) {
	if(!upperCase) {
		StringUtils::uIntToHexBuf<false>(num, digits, buf);
	}
	else {
		StringUtils::uIntToHexBuf<true>(num, digits, buf);
	}	
}

void StringUtils::uIntToNumBaseBuf(uint64_t num, uint8_t digits, char* buf, uint8_t base, bool upperCase) {
	char* bufPtr = buf + digits;
	while (digits--) {
		uint8_t cNum = num % base;
		char c = '#';
		if (cNum <= 9)
			c = '0' + cNum;
		else if (cNum <= 36) {
			c = (upperCase ? 'A' : 'a') + (cNum - 10);
		}
		*--bufPtr = c;
		num /= base;
	}
}

std::string StringUtils::uIntToNumBaseStr(uint64_t num, uint8_t digits, uint8_t base, bool upperCase) {
	std::string s(digits, ' ');
	uIntToNumBaseBuf(num, digits, &s[0], base, upperCase);
	return s;
}

void StringUtils::uIntToBinBuf(uint64_t num, uint8_t digits, char* buf) {
	char* bufPtr = buf + digits;
	for (uint8_t i = 0; i < digits; i++) {
		*--bufPtr = num & 1 ? '1' : '0';
		num >>= 1;
	}
}
std::string StringUtils::uIntToBinStr(uint64_t num, uint8_t digits) {
	std::string s(digits, ' ');
	uIntToBinBuf(num, digits, &s[0]);
	return s;
}

std::string StringUtils::paddLeft(const std::string& s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.begin(), paddedLength - out.size(), paddWith);
	return out;
}
std::string StringUtils::paddRight(const std::string& s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.end(), paddedLength - out.size(), paddWith);
	return out;
}

std::string StringUtils::loadFileIntoString(const char* path, bool* success) {
	std::ifstream t(path);
	std::string fileStr;

	if(!t.is_open()){
		if (success)
			*success = false;
		return "";
	}

	fileStr = "";

	t.seekg(0, std::ios::end);
	uint64_t size = t.tellg();
	t.seekg(0, std::ios::beg);
	fileStr.resize((size_t)size);

	t.read(&fileStr[0], size);
	t.close();
	if (success)
		*success = true;
	return fileStr;
}

bool StringUtils::writeStringToFile(const std::string& str, const char* path) {
	std::ofstream out(path);
	out << str;
	out.close();
	return true;
}

std::vector<uint8_t> StringUtils::loadFileIntoByteArray(const char* path, bool* success) {
	std::ifstream t(path, std::ios::binary);
	std::vector<uint8_t> byteArr;

	if(!t.is_open()){
		if (success)
			*success = false;
		return byteArr;
	}

	byteArr.clear();

	t.seekg(0, std::ios::end);
	uint64_t size = t.tellg();
	t.seekg(0, std::ios::beg);
	byteArr.resize((size_t)size);

	t.read((char*) & byteArr[0], size);
	t.close();

	if (success)
		*success = true;

	return byteArr;
}

bool StringUtils::writeBytesToFile(const uint8_t* data, size_t dataLen, const char* path) {
	std::ofstream out(path, std::ios::binary);
	out.write((const char*)data, dataLen);
	out.close();
	return true;
}



size_t StringUtils::findCharInStr(char c, const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);
	for (const char* ptr = str; ptr < strEnd; ptr++) {
		if (*ptr == c)
			return ptr - str;
	}
	return -1;
}
size_t StringUtils::findCharInStrFromBack(char c, const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);
	for (const char* ptr = strEnd; ptr != str; ptr--) {
		if (*ptr == c)
			return ptr - str;
	}
	return -1;
}

std::vector<std::pair<size_t,std::string>> StringUtils::findStrings(const uint8_t* data, size_t dataLen, size_t minLen) {
	std::vector<std::pair<size_t,std::string>> out;
	size_t runStart = 0;
	size_t runCnt = 0;
	for (size_t i = 0; i < dataLen; i++) {
		if (isprint(data[i])) {
			if (runCnt == 0)
				runStart = i;
			runCnt++;
		}
		else {
			if (data[i] == 0 && runCnt >= minLen) {
				out.push_back({ runStart, std::string((const char*)data + runStart, (const char*)data + runStart + runCnt) });
			}
			runCnt = 0;
		}
	}
	return out;
}

int StringUtils::strcasecmp(const char* a, const char* b, const char* a_end, const char* b_end) {
	if (a_end == NULL)
		a_end = a + strlen(a);
	if (b_end == NULL)
		b_end = b + strlen(b);

	if (a_end - a != b_end - b) {
		return a_end - a > b_end - b ? 1 : -1;
	}
	
	while (true) {
		if (a == a_end || b == b_end) {
			if (a == a_end && b == b_end)
				return 0;

			return b == b_end ? 1 : -1;
		}

		char ac = *a;
		char bc = *b;
		if (ac >= 'A' && ac <= 'Z')
			ac += 'a' - 'A';
		if (bc >= 'A' && bc <= 'Z')
			bc += 'a' - 'A';

		if (ac != bc) {
			return ac - bc;
		}

		a++;
		b++;
	}

	return 0;
}


uint64_t StringUtils::stof(const char* str, const char* str_end, uint8_t exponentBits, uint8_t fractionBits, bool atof_compatible) {
	if (str_end == NULL)
		str_end = str + strlen(str);


	const uint64_t exponentBias = (1 << exponentBits) / 2 - 1;


	const char* str_stripped = str;
	const char* str_end_stripped = str_end;

	uint64_t sign = 0, exponent = exponentBias, fraction = 0;
	

	while (isspace(*str_stripped))
		str_stripped++;

	if (str_stripped >= str_end) // return NaN if string is just empty (or just whitespace)
		if (atof_compatible)
			goto stof_zero;
		else
			goto stof_nan;

	while (str_end_stripped > str_stripped && isspace(*(str_end_stripped-1)))
		str_end_stripped--;

	// maybe check something here??

	if (!atof_compatible && strcasecmp(str_stripped, "inf", str_end_stripped) == 0) {
		goto stof_inf;
	}

	if (!atof_compatible && strcasecmp(str_stripped, "nan", str_end_stripped) == 0) {
		goto stof_nan;
	}

	{
		const char* str_w = str_stripped;
		const char* str_end_w = str_end_stripped;

		if (*str_w == '+') {
			sign = 0;
			str_w++;
		}
		else if (*str_w == '-') {
			sign = 1;
			str_w++;
		}

		uint64_t digits = 0, decimals = 0;
		int exponentDec = 0;
		size_t decimalsSeqLen = 0;

		// extract digits
		{
			bool skip = false;
			const char* digitSeqEnd = str_w;
			while (true) {
				if (digitSeqEnd >= str_end_stripped) {
					skip = true;
					break;
				}

				if (!isdigit(*digitSeqEnd))
					break;

				digitSeqEnd++;
			}

			size_t digitSeqLen = digitSeqEnd - str_w;
			if (digitSeqLen > 0) {
				uint64_t n = 0;
				for (size_t i = 0; i < digitSeqLen; i++) {
					uint64_t n_ = n * 10;

					if (n_ / 10 != n) { // overflow
						// TODO: round
						exponentDec += (int)digitSeqLen - (int)i;
						goto stof_digits_done;
					}
					
					n = n_ + (str_w[i] - '0');
				}
stof_digits_done:
				digits = n;
			}
			str_w = digitSeqEnd;

			if(skip)
				goto stof_calc;
		}

		// at this point str_w is guaranteed to be < str_end_stripped

		// extract decimals
		if (*str_w == '.') {
			str_w++;

			bool skip = false;

			const char* decimalsSeqEnd = str_w;
			while (true) {
				if (decimalsSeqEnd >= str_end_stripped) { // check if at end of string
					skip = true;
					break;
				}

				if (!isdigit(*decimalsSeqEnd))
					break;

				decimalsSeqEnd++;
			}


			decimalsSeqLen = decimalsSeqEnd - str_w;
			if (decimalsSeqLen > 0) {
				uint64_t n = 0;
				for (size_t i = 0; i < decimalsSeqLen; i++) {
					uint64_t n_ = n * 10;

					if (n_*10 / 100 != n) { // check for overflow in the next round
						if (str_w[i] - '0' >= 5)
							n++; // round
						decimalsSeqLen = i;
						goto stof_decimals_done;
					}

					n = n_ + (str_w[i] - '0');
				}
stof_decimals_done:
				decimals = n;
			}

			str_w = decimalsSeqEnd;

			if(skip)
				goto stof_calc;
		}

		// at this point str_w is guaranteed to be < str_end_stripped

		// extract exponent
		if (*str_w == 'e' || *str_w == 'E') {
			str_w++;

			if (str_w >= str_end_stripped) // illegal
				goto stof_nan;

			bool expIsNeg = false;

			if (*str_w == '+') {
				expIsNeg = false;
				str_w++;
			}
			else if (*str_w == '-') {
				expIsNeg = true;
				str_w++;
			}

			const char* exponentSeqEnd = str_w;
			while (true) {
				if (exponentSeqEnd >= str_end_stripped) // check if at end of string
					break;

				if (!isdigit(*exponentSeqEnd))
					break;

				exponentSeqEnd++;
			}

			if (exponentSeqEnd > str_w) {
				int expOff = 0;
				for (const char* s = str_w; s < exponentSeqEnd; s++) { // convert dec string to int
					expOff = expOff * 10 + (*s - '0');
				}
				exponentDec += expIsNeg ? -expOff : expOff;
			}

			str_w = exponentSeqEnd;
		}


		if (str_w < str_end_w) {
			if (*str_w == 'f' || *str_w == 'F' || *str_w == 'l' || *str_w == 'L')
				str_w++;
		}

		if (!atof_compatible && str_w != str_end_w) // check if there is still something left in the string (illegal, if not atof_compadible)
			goto stof_nan;

stof_calc:
		{
			int exp = 0;

			uint8_t digitsHBS = getHBS(digits);


			uint64_t decimalsBin = 0; // binary representation of fraction, but left aligned (msb set = 0.5)
			uint8_t decimalsBinInd = 0; // basically the length
			if (decimals != 0) {
				if (digits == 0) {
					digits = decimals;
					decimals = 0;
					exponentDec -= (int)decimalsSeqLen;
				}
				else {
					if(digitsHBS < fractionBits) {
						uint64_t decimals_cpy = decimals;
						uint8_t decimalsBinIndFirst1 = -1;

						uint64_t decimalsUpperBound = 1;
						for (size_t i = 0; i < decimalsSeqLen; i++) {
							uint64_t n = decimalsUpperBound * 10;
							if (n / 10 != decimalsUpperBound) {
								goto stof_zero;
							}
							decimalsUpperBound = n;
						}

						while (decimals_cpy > 0) {
							decimals_cpy <<= 1;
							if (decimals_cpy >= decimalsUpperBound) {
								decimals_cpy -= decimalsUpperBound;
								decimalsBin |= (uint64_t)1 << (64 - decimalsBinInd - 1);

								if (decimalsBinIndFirst1 == (uint8_t)-1)
									decimalsBinIndFirst1 = decimalsBinInd;
							}
							decimalsBinInd++;

							if (decimalsBinIndFirst1 != (uint8_t)-1 && decimalsBinInd - decimalsBinIndFirst1 > fractionBits - digitsHBS + 1) // check if enough bits were read
								break;
						}
					}
				}
			}
			
			
			//exp += digitsHBS;
			exp -= decimalsBinInd;

			uint64_t combined = (digits << decimalsBinInd) | (decimalsBin >> (64-decimalsBinInd)); // right aligned (normal)

			if (combined == 0)
				goto stof_zero;
			else if (exponentDec != 0) {
				while (!(combined & 1)) {
					combined >>= 1;
					exp++;
				}

				exp += exponentDec; // get the prime factor 2 out of the base 10
				

				if (exponentDec > 0) {
					for (int i = 0; i < exponentDec; i++) {
						uint64_t n;
						while (true) {
							n = combined * 5;
							if (n / 5 != combined) { // overflow detected
								combined >>= 1;
								exp++;
								continue;
							}
							break;
						}
						combined = n;
					}
					while (!(combined & 1)) {
						combined >>= 1;
						exp++;
					}
				}
				else {
					while (!(combined & ((uint64_t)1 << 63))) { // left align
						combined <<= 1;
						exp--;
					}
					for (int i = 0; i < -exponentDec; i++) {
						combined /= 5;
						while (!(combined & ((uint64_t)1 << 63))) {
							combined <<= 1;
							exp--;
						}
					}
					while (!(combined & 1)) { // right align again
						combined >>= 1;
						exp++;
					}
				}
			}

			{
				uint8_t comb_hbs = getHBS(combined);

				exp += comb_hbs;


				uint64_t comb_fraction = combined;
				if (comb_hbs <= fractionBits) { // shift to fit into fraction of set bitlength
					comb_fraction <<= fractionBits - comb_hbs;
				}
				else {
					uint8_t shft_amt = comb_hbs - fractionBits;
					comb_fraction >>= shft_amt - 1; // shift right by one less

					uint8_t roundUp = comb_fraction & 1; // get last bit for rounding
					comb_fraction >>= 1; // shift right by the remaining 1
					comb_fraction += roundUp; // TODO: check for overflow maybe?
				}

				fraction = comb_fraction & (((uint64_t)1<<fractionBits)-1); // shear off the highest bit set, since thats always set (by the standard)
			}
			

			if ((int64_t)exponentBias + exp < 0) // exponent too small
				goto stof_zero;
			else if (exponentBias + exp >= ((uint64_t)1 << exponentBits)) // exponent too big
				goto stof_inf;
			else
				exponent = exponentBias + exp;
			
			goto stof_end;
		}
	}

stof_inf:
	exponent = (1 << exponentBits) - 1; // all bits set
	fraction = 0;
	goto stof_end;

stof_nan:
	exponent = (1 << exponentBits) - 1; // all bits set
	fraction = 1;
	goto stof_end;

stof_zero:
	exponent = 0;
	fraction = 0;
	goto stof_end;

stof_end:
	return (sign << (exponentBits + fractionBits)) | (exponent << fractionBits) | fraction;
}

uint8_t StringUtils::getLBS(uint64_t x) {
	if (x == 0) return 64;

	uint8_t ret = 0;
	while ((x & 1) == 0) {
		x >>= 1;
		ret++;
	}
	return ret;
}
uint8_t StringUtils::getHBS(uint64_t x) {
	uint8_t ret = 0;
	while (x >>= 1) ret++;
	return ret;
}

const char* StringUtils::getFileName(const char* path, const char* path_end) {
	if (path_end == 0)
		path_end = path + strlen(path);

	size_t lastSlash = findCharInStrFromBack('/', path, path_end);
	size_t lastBSlash = findCharInStrFromBack('\\', path, path_end);
	size_t lastDiv = std::max(lastSlash != (size_t)-1 ? lastSlash : 0, lastBSlash != (size_t)-1 ? lastBSlash : 0);

	return path + lastDiv + 1;
}
const char* StringUtils::getFileExtension(const char* path, const char* path_end) {
	if (path_end == 0)
		path_end = path + strlen(path);

	size_t lastSlash = findCharInStrFromBack('/', path, path_end);
	size_t lastBSlash = findCharInStrFromBack('\\', path, path_end);
	size_t lastDiv = std::max(lastSlash != (size_t)-1 ? lastSlash : 0, lastBSlash != (size_t)-1 ? lastBSlash : 0);

	size_t lastDot = findCharInStrFromBack('.', path, path_end);

	if (lastDot > lastDiv) {
		return path + lastDot + 1;
	}
	else {
		return path_end;
	}
}

std::vector<size_t> StringUtils::generateLineIndexArr(const char* str) {
	std::vector<size_t> lines;

	const char* ptr = str;
	lines.push_back(0);
	while (*ptr) {
		char c = *ptr;
		if (c == '\n') {
			size_t line = (ptr - str) + 1;
			lines.push_back(line);
		}
		ptr++;
	}

	return lines;
}



std::string StringUtils::addThousandsSeperator(const char* str, const char* str_end, const char* seperator) {
	if (str_end == nullptr)
		str_end = str + std::strlen(str);

	size_t len = str_end - str;
	if (len == 0) return "";

	size_t sep_len = std::strlen(seperator);
	if (sep_len == 0) return str;

	size_t num_seps = (len-1) / 3;
	std::string out(len + num_seps * sep_len , 'A');

	for (size_t i = 0; ; i++) {
		size_t sub_l = std::min((size_t)3, len - i * 3);
		std::memcpy(&out[0]+(out.size()-(i * (3 + sep_len))-sub_l), str_end - (i*3 + sub_l), sub_l); // copy over 3 chars of str

		if (i >= num_seps)
			break;

		std::memcpy(&out[0]+(out.size()-(i * (3 + sep_len))-3-1), seperator, sep_len); // copy over seperator
	}

	return out;
}

/*














uint64_t StringUtils::stof(const char* str, const char* str_end, uint8_t exponentBits, uint8_t fractionBits) {
if (str_end == NULL)
str_end = str + strlen(str);


const uint64_t exponentBias = (1 << exponentBits) / 2 - 1;


const char* str_stripped = str;
const char* str_end_stripped = str_end;

uint64_t sign = 0, exponent = exponentBias, fraction = 0;

while (isspace(*str_stripped))
str_stripped++;

if (str_stripped >= str_end) // return NaN if string is just empty (or just whitespace)
goto stof_nan;

while (str_end_stripped < str_stripped && isspace(*str_end_stripped))
str_end_stripped--;

// maybe check something here??

if (strcasecmp(str_stripped, "inf", str_end_stripped) == 0) {
goto stof_inf;
}

if (strcasecmp(str_stripped, "nan", str_end_stripped) == 0) {
goto stof_nan;
}

{
const char* str_w = str_stripped;
const char* str_end_w = str_end_stripped;

if (*str_w == '+') {
sign = 0;
str_w++;
}
if (*str_w == '-') {
sign = 1;
str_w++;
}

uint64_t digits = 0, decimals = 0;
int exponentDec = 0;
size_t decimalsSeqLen = 0;
int exp = 1;

// extract digits
{
const char* digitSeqEnd = str_stripped;
while (true) {
if (digitSeqEnd >= str_end_stripped) {
digits = numBaseStrToUIntT<10>(str_stripped, digitSeqEnd);
goto stof_calc;
}

if (!isdigit(*digitSeqEnd))
break;

digitSeqEnd++;
}

digits = numBaseStrToUIntT<10>(str_stripped, digitSeqEnd);
str_w = digitSeqEnd;
}

// at this point str_w is guaranteed to be < str_end_stripped

// extract decimals
if (*str_w == '.') {
str_w++;

const char* decimalsSeqEnd = str_w;
while (true) {
if (decimalsSeqEnd >= str_end_stripped) {
if (decimalsSeqEnd > str_w) {
decimals = numBaseStrToUIntT<10>(str_w, decimalsSeqEnd);
decimalsSeqLen = decimalsSeqEnd - str_w;
}

goto stof_calc;
}

if (!isdigit(*decimalsSeqEnd))
break;

decimalsSeqEnd++;
}

if (decimalsSeqEnd > str_w) {
decimals = numBaseStrToUIntT<10>(str_w, decimalsSeqEnd);
decimalsSeqLen = decimalsSeqEnd - str_w;
}

str_w = decimalsSeqEnd;
}

// at this point str_w is guaranteed to be < str_end_stripped

// extract exponent
if (*str_w == 'e' || *str_w == 'E') {
str_w++;

if (str_w >= str_end_stripped) // illegal
goto stof_nan;

bool expIsNeg = false;

if (*str_w == '+') {
expIsNeg = false;
str_w++;
}
else if (*str_w == '-') {
expIsNeg = true;
str_w++;
}

const char* exponentSeqEnd = str_w;
while (true) {
if (exponentSeqEnd >= str_end_stripped)
break;

if (!isdigit(*exponentSeqEnd))
break;

exponentSeqEnd++;
}

if (exponentSeqEnd > str_w) {
exponentDec = numBaseStrToUIntT<10>(str_w, exponentSeqEnd);
if (expIsNeg)
exponentDec = -exponentDec;
}

str_w = exponentSeqEnd;
}


if (str_w < str_end_w) {
if (*str_w == 'f' || *str_w == 'F')
str_w++;
}

if (str_w != str_end_w) // illegal
goto stof_nan;

stof_calc:
{

int exp = 0;

uint8_t digitsHBS = getHBS(digits);


uint64_t decimalsBin = 0; // binary representation of fraction, but left aligned (msb = 0.5)
uint8_t decimalsBinInd = 0; // basically the length
{
uint64_t decimals_cpy = decimals;

uint64_t decimalsUpperBound = pow(10, decimalsSeqLen);
while (decimalsBinInd <= fractionBits - digitsHBS && decimals_cpy > 0) {
decimals_cpy <<= 1;
if (decimals_cpy >= decimalsUpperBound) {
decimals_cpy -= decimalsUpperBound;
decimalsBin |= (uint64_t)1 << (64 - decimalsBinInd - 1);
}

decimalsBinInd++;
}
}


//exp += digitsHBS;
exp -= decimalsBinInd;


uint64_t combined = (digits << decimalsBinInd) | (decimalsBin >> (64-decimalsBinInd)); // right aligned (normal)
while (!(combined & 1)) {
combined >>= 1;
exp++;
}

if (combined == 0)
exponent = 0;
else if (exponentDec != 0) {
exp += exponentDec; // get the prime factor 2 out of the base 10


if (exponentDec > 0) {
for (int i = 0; i < exponentDec; i++) {
uint64_t n;
while (true) {
n = combined * 5;
if (n / 5 != combined) { // overflow detected
combined >>= 1;
exp++;
continue;
}
break;
}
combined = n;
}
while (!(combined & 1)) {
combined >>= 1;
exp++;
}
}
else {
while (!(combined & ((uint64_t)1 << 63))) { // left align
combined <<= 1;
exp--;
}
for (int i = 0; i < -exponentDec; i++) {
combined /= 5;
while (!(combined & ((uint64_t)1 << 63))) {
combined <<= 1;
exp--;
}
}
while (!(combined & 1)) { // right align again
combined >>= 1;
exp++;
}
}
}

uint8_t comb_hbs = getHBS(combined);

exp += comb_hbs;


uint64_t comb_fraction = combined;
if (comb_hbs <= fractionBits) { // shift to fit into fraction of set bitlength
comb_fraction <<= fractionBits - comb_hbs;
}
else {
uint8_t shft_amt = comb_hbs - fractionBits;
comb_fraction >>= shft_amt - 1; // shift right by one less

uint8_t roundUp = comb_fraction & 1; // get last bit for rounding
comb_fraction >>= 1; // shift right by the remaining 1
comb_fraction += roundUp;
}

fraction = comb_fraction & (1<<fractionBits)-1; // shear off the highest bit set, since thats always set (by the standard)

if (exponent != 0)
if (exponentBias + exp >= 0 && exponentBias + exp < ((uint64_t)1 << exponentBits))
exponent = exponentBias + exp;
else
goto stof_inf;


goto stof_end;
}
}

stof_inf:
exponent = (1 << exponentBits) - 1; // all bits set
fraction = 0;
goto stof_end;

stof_nan:
exponent = (1 << exponentBits) - 1; // all bits set
fraction = 1;
goto stof_end;

stof_end:
return (sign << (exponentBits + fractionBits)) | (exponent << fractionBits) | fraction;
}















fraction = (combined << (comb_hbs <= fractionBits ? fractionBits - getHBS(combined) : 0)) & (1<<fractionBits)-1; // shear off the highest bit set, since thats always set (by the standard)



if (combined << exp != 1) {
if (combined << exp == 0) {
while (combined << exp != 1) {
exp++;
}
}
else {
while (combined << exp != 1) {
exp--;
}
}
}



fraction = (combined >> (64 - fractionBits - 1)) & (1<<fractionBits)-1; // -1 to shear off the highest bit set, since thats always set (by the standard)




uint8_t decimalsHBS = getHBS(decimalsBin);
decimalsHBS = 64 - decimalsHBS - 1;

if (digits > 0) {
exp += digitsHBS;
exp -= getLBS()
}
else {
exp -= decimalsHBS + 1;
}







uint8_t decimalsHBS = getHBS(decimalsBin);
decimalsHBS = 64 - decimalsHBS - 1;

uint64_t combinedLen;
if (digitsHBS > 0) {
exponent += digitsHBS;
combinedLen = decimalsBinInd + digitsHBS - 1;
}
else {
exponent -= decimalsHBS + 1;
combinedLen = decimalsHBS - 1;
}
uint64_t combined = (digits << decimalsBinInd) | (decimalsBin >> (64-decimalsBinInd-1)); // right aligned (normal)







else {
bool expIsNeg = false;
if (exponentDec < 0) {
exponentDec = -exponentDec;
expIsNeg = true;
}

uint64_t exponentDecMul = pow(10, exponentDec);

while ((exponentDecMul & 1) == 0) {
exponentDecMul >>= 1;
exponent++;
}

if (expIsNeg) {
uint8_t hbs = getHBS(exponentDecMul);
uint64_t nextPow2 = hbs + 1;
fraction = ((fraction << nextPow2) / exponentDecMul)>>nextPow2;

exponent = exponentBias - exponent;
}	
else {
combined *= exponentDecMul;
exponent += exponentBias;
}

combinedLen = getHBS(combined);
}









uint64_t StringUtils::hexToUInt(const char* str, const char* strEnd){
if (strEnd == nullptr)
strEnd = str + std::strlen(str);

uint64_t num = 0;
const char* strPtr = str;
while (strPtr != strEnd) {
const char c = *strPtr++;
uint8_t cNum = -1;
if (c >= '0' && c <= '9')
cNum = c - '0';
else {
if (c >= 'A' && c <= 'F')
cNum = c - 'A' + 10;
else if (c >= 'a' && c <= 'f')
cNum = c - 'a' + 10;
else
return -1;
}
num <<= 4;
num |= cNum;
}
return num;
}


std::string stringExtras::intToBin(uint16_t val, uint8_t digits) {
std::string out = "";
for (uint8_t i = 0; i < digits; i++) {
out = hexDigits[val & 0b1] + out;
val >>= 1;
}
return out;
}
std::string stringExtras::intToHex(uint16_t val, uint8_t digits) {
std::string out = "";
for (uint8_t i = 0; i < digits; i++) {
out = hexDigits[val & 0xF] + out;
val >>= 4;
}
return out;
}

uint8_t stringExtras::HexDigitToInt(char digit) {
if(digit >= 'a' && digit <= 'z') {
digit -= ('a'-'A');
}
for (uint8_t ind = 0; ind < 16; ind++) {
if (digit == hexDigits[ind]) {
return ind;
}
}
if(digit == ' ')
return 0;
return 255;
}

uint32_t stringExtras::HexStrToInt(const char * str) {
size_t len = std::strlen(str);
uint32_t out = 0;
for (size_t i = 0; i < len; i++) {
out <<= 4;
out |= HexDigitToInt(*(str + i));
}
return out;
}

*/