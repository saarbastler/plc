#ifndef _INCLUDE_PLC_H_
#define _INCLUDE_PLC_H_

#include <PlcAst.h>
#include <SvgOption.h>

void plcParse(std::istream& in, PlcAst& plcAst);
void plcParse(const char *text, PlcAst& plcAst);

void convert2svg(const PlcAst& plcAst, const plc::Expression& expression, const std::string& name, std::ostream& out, const std::initializer_list<SVGOption> options);
void convert2svg(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<SVGOption> options);

void translate2Avr(const PlcAst& plcAst, std::vector<uint8_t>& avrplc);

#endif // !_INCLUDE_PLC_H_

