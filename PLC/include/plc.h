#ifndef _INCLUDE_PLC_H_
#define _INCLUDE_PLC_H_

#include <PlcAst.h>

void plcParse(std::istream& in, PlcAst& plcAst);
void plcParse(const char *text, PlcAst& plcAst);

void convert2svg(const PlcAst& plcAst, const plc::Expression& expression, std::ostream& out);

#endif // !_INCLUDE_PLC_H_

