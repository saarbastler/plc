#if !defined(PARSER_TESTS) && !defined(LIBRARY)

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "PlcParser.h"
#include "plc2svg.h"
#include "PlcCompiler.h"
#include "plc.h"

namespace po = boost::program_options;

#define INPUT_FILE_NAME   "plc-file"
#define INPUT_FILE        INPUT_FILE_NAME ",P"

#define OUTPUT_FILE_NAME  "output-file"
#define OUTPUT_FILE       OUTPUT_FILE_NAME ",O"

#define LIST_NAME         "list"
#define LIST              LIST_NAME ",L"

#define ALL_NAME          "all"
#define ALL               ALL_NAME ",A"

#define OUTPUTS_NAME      "outputs"
#define OUTPUTS           OUTPUTS_NAME ",o"

#define EQUATION_NAME     "equation"
#define EQUATION          EQUATION_NAME ",E"

#define INTERACTIVE_NAME  "interactive"
#define INTERACTIVE       INTERACTIVE_NAME ",I"

#define NO_JS_NAME        "nojs"
#define NO_JS             NO_JS_NAME ",J"

#define RESOLVE_DEP_NAME  "resolve"
#define RESOLVE_DEP       RESOLVE_DEP_NAME ",R"

#define LINK_LABELS_NAME  "labels"
#define LINK_LABELS       LINK_LABELS_NAME

#define BOX_TEXT_NAME     "box-text"
#define BOX_TEXT          BOX_TEXT_NAME ",T"

#define USAGE             "Usage: plc [options] plc-file\n  plc -L plcfile\n  plc -E test -O out.svg plcfile\n"

class OptionsException : public std::exception
{
public:

  OptionsException(const char *msg) : msg(msg) {}

  virtual const char *what() const noexcept
  {
    return msg;
  }

private:
  const char *msg;
};

int list(const std::string& inputfile, bool onlyOutputs)
{
  std::ifstream in(inputfile);

  PlcAst plcAst;
  try
  {
    plcParse(in, plcAst);

    for (auto it = plcAst.variableDescription().begin(); it != plcAst.variableDescription().end(); it++)
      if (!onlyOutputs ||
        (onlyOutputs && it->second.type() == Variable::Type::Output))
      {
        std::cout << it->first << std::endl;
      }
  }
  catch (std::exception& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;

    return 1;
  }
  return 0;
}

int equation(const po::variables_map& vm)
{
  if (!vm.count(OUTPUT_FILE_NAME))
    throw OptionsException("missing output file name");

  PlcAst plcAst;
  try
  {
    std::ifstream in(vm[INPUT_FILE_NAME].as<std::string>());

    plcParse(in, plcAst);
  }
  catch (std::exception& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;

    return 1;
  }

  std::ofstream out(vm[OUTPUT_FILE_NAME].as<std::string>());

  std::vector<SVGOption> options;
  if (vm.count(NO_JS_NAME))
    options.emplace_back(SVGOption::NoJavascript);
  else if (!vm.count(INTERACTIVE_NAME))
    options.emplace_back(SVGOption::NotInteractive);
  if (vm.count(LINK_LABELS_NAME))
    options.emplace_back(SVGOption::LinkLabels);
  if (vm.count(BOX_TEXT_NAME))
    options.emplace_back(SVGOption::BoxText);

  Plc2svg plc2svg(plcAst, out, options);
  if (vm.count(ALL_NAME))
  {
    std::vector<std::string> names;
    for (auto it = plcAst.variableDescription().begin(); it != plcAst.variableDescription().end(); it++)
      if (plcAst.getVariable(it->first).type() == Variable::Type::Output && it->second.expression().operator bool())
        names.emplace_back(it->first);

    plc2svg.convertMultiple(names);
  }
  else
  {
    const std::string& equationName = vm[EQUATION_NAME].as<std::string>();
    const Variable& variable(plcAst.getVariable(equationName));
    if (!variable.expression().operator bool())
    {
      std::cout << "Error: Equation " << equationName << " does not exists.";

      return 1;
    }

    const plc::Expression& equation = *variable.expression();

    if (vm.count(RESOLVE_DEP_NAME))
      plc2svg.convert(plcAst.resolveDependencies(equationName), equationName);
    else
      plc2svg.convert(equation, equationName);
  }

  return  0;
}

int main(int argc, char *argv[])
{
  po::options_description desc("Options");
  desc.add_options()
    ( "help,?", "Show Help")
    ( INPUT_FILE, po::value<std::string>()->required(), "plc file")
    ( OUTPUT_FILE, po::value<std::string>(), "output file")
    ( EQUATION, po::value<std::string>(), "convert a single Equation")
    ( ALL, "convert all Equations")
    ( LIST, "list Equations")
    ( OUTPUTS, "list only Outputs")
    ( INTERACTIVE, "Interactive SVG generation")
    ( NO_JS, "no Javascript at all")
    ( BOX_TEXT, "addition text field in each box")
    ( LINK_LABELS, "Label link wires")
    ( RESOLVE_DEP, "resolve Dependencies in SVG generation")
    ;

  po::variables_map vm;
  po::positional_options_description p;
  p.add(INPUT_FILE_NAME, -1);

  try
  {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << USAGE << desc;
      return 0;
    }

    if (vm.count(LIST_NAME) + vm.count(EQUATION_NAME) + vm.count(ALL_NAME) > 1)
      throw OptionsException("Only one Option of " LIST_NAME ", " EQUATION_NAME " or " ALL_NAME " accepted.");

    if (vm.count(LIST_NAME))
      return list(vm[INPUT_FILE_NAME].as<std::string>(), vm.count(OUTPUTS_NAME) > 0);
    else if (vm.count(EQUATION_NAME) || vm.count(ALL_NAME))
      return equation(vm);
    else
      throw OptionsException("at least one Option of " LIST_NAME ", " EQUATION_NAME " or " ALL_NAME " necessary");
  }
  catch (const std::exception& ex)
  {
    std::cout << "Error parsing Commandline Arguments: " << ex.what() << std::endl
      << USAGE << desc;

    return 1;
  }

    /*std::vector<plc::Operation> instructions;
    plc::compile(plcAst, equation, "output", instructions);

    PlcSimulator plcSimulator(5, 0, 0, 0);

    for (unsigned bitvector = 0; bitvector < 32; bitvector++)
    {
      for (int i = 4; i >= 0; i--)
      {
        plcSimulator.io(PlcSimulator::IOType::Input, i) = bitvector & (1 << i);

        std::cout << (plcSimulator.io(PlcSimulator::IOType::Input, i) ? 1 : 0) << ' ';
      }

      std::cout << " = " << (plcSimulator.execute<5>(instructions) ? 1 : 0) << std::endl;
    }

    std::vector<uint8_t> avrplc;
    plc::translateAvr(instructions, avrplc);

    std::cout.width(2);
    std::cout.fill('0');
    for (uint8_t op : avrplc)
      std::cout << std::hex << unsigned(op) << ' ';

    std::cout << std::endl;*/

  return 0;
}

#endif
