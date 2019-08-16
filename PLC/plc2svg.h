#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>
#include <sstream>
#include <unordered_map>

#include "plc2svgbase.h"

class Plc2svg : public Plc2svgBase
{
public:

  Plc2svg(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<SVGOption> options) : Plc2svgBase(plcAst, out, options)
  {
  }

  template<typename AT>
  Plc2svg(const PlcAst& plcAst, std::ostream& out, const AT& options) : Plc2svgBase(plcAst, out, options)
  {
  }

  void convertMultiple(const std::vector<std::string>& names)
  {
    std::unordered_map<std::string, plc::Expression> resolved;
    std::unordered_map<std::string,unsigned> toSkip;

    for (auto it = names.begin(); it != names.end(); it++)
      resolved[*it]= plcAst.resolveDependencies(*it, &toSkip);

    for (auto it = toSkip.begin(); it != toSkip.end(); it++)
      if (it->second > 1)
        signalCrossing.emplace(it->first);

    for (auto it = names.begin(); it != names.end(); it++)
      resolved[*it].countInputs(inputCountMap);

    setupParameter(resolved, names);

    for (auto it = names.begin(); it != names.end(); it++, yposLast++)
      convert(yposLast + 1, 0, resolved[*it], plc::Term::Unary::None, &plcAst.getVariable(*it));

    writeOutput();
  }

  void convert(const plc::Expression& expression, const std::string& name)
  {
    expression.countInputs(inputCountMap);

    std::vector<std::string> names(1, name);
    setupParameter(plcAst.equations(), names);

    maxLevel = expression.countLevels();

    const Variable& variable = plcAst.getVariable(name);
    convert(1, 0, expression, plc::Term::Unary::None, &variable);

    writeOutput();
  }

private:

  void countLevelCrossings(unsigned level, const plc::Expression& expression)
  {
    while (crossingsPerLevel.size() <= level)
      crossingsPerLevel.emplace_back(0);

    if (!expression.signalName().empty())
    {
      auto find = inputCountMap.find(expression.signalName());
      if (find != inputCountMap.end())
        ++crossingsPerLevel[level];
    }

    for (auto it = expression.terms().begin(); it != expression.terms().end(); it++)
      if (it->type() == plc::Term::Type::Expression)
        countLevelCrossings(level + 1, *it->expression());
  }

  void setupParameter(const std::unordered_map<std::string, plc::Expression>& expressions, const std::vector<std::string> names)
  {
    maxLevel = 0;
    for (auto it = names.begin(); it != names.end(); it++)
    {
      countLevelCrossings(0, expressions.at(*it));
      unsigned tmp = expressions.at(*it).countLevels();
      if (tmp > maxLevel)
        maxLevel = tmp;
    }

    unsigned crossings = 0;
    unsigned chars = 0;
    for (auto it = inputCountMap.begin(); it != inputCountMap.end(); it++)
    {
      unsigned width = unsigned(it->first.length());
      if (width > chars)
        chars = width;

      if (it->second > 1)
        crossings++;
    }

    crossingsPerLevel.emplace_back(crossings);

    textWidth = chars * CHAR_WIDTH;
  }

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression, plc::Term::Unary unary, const Variable *variable = nullptr)
  {
    //std::cout << "convert Expression " << expression.signalName() << " " << expression.id() << " ypos: " << ypos << " level: " << level << std::endl;

    if (ypos > yposLast)
      yposLast = ypos;
    
    unsigned size = 0;
    unsigned index = 1;
    unsigned width = textWidth + (maxLevel - level - 1) * (LINE_LENGTH + GATE_WIDTH) + LINE_LENGTH;
    unsigned outy = (index + ypos) * CHAR_HEIGHT;

    for (const plc::Term& term : expression.terms())
    {
      unsigned y = (index + ypos) * CHAR_HEIGHT;
      if (index + ypos > yposLast)
        yposLast = index + ypos;

      switch (term.type())
      {
      case plc::Term::Type::Identifier:
        convertInput(term, level, y, width);

        index++;
        size++;
        break;

      case plc::Term::Type::Expression:
      {
        unsigned subSize = 2 + convert(index + ypos, level + 1, *term.expression(), term.unary());

        index += subSize;
        size += subSize;
      }
      break;

      //default:
      }
    }

    unsigned lineX1 = crossingWidthUpToLevel(level) + width;
    unsigned lineX2 = crossingWidthUpToLevel(level - 1) + width + GATE_WIDTH + LINE_LENGTH;

    if (unary != plc::Term::Unary::None)
      lineX2 -= 2 * INVERT_RADIUS;

    if (expression.op() != plc::Expression::Operator::None /*|| expression.terms().size() > 1*/)
    {
      svgOut << svg::Rect(lineX1, ypos * CHAR_HEIGHT, GATE_WIDTH, (size + 1) * CHAR_HEIGHT, { BOX })
        << svg::Text(lineX1 + CHAR_OFFSET, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, operatorSymbol(expression.op()), {});

      if ( hasOption(SVGOption::BoxText))
      {
        std::ostringstream tid;
        tid << 't' << gateCssClass(expression);

        svgOut << svg::Text(lineX1 + CHAR_OFFSET, (2 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, "", {}, tid.str().c_str());
      }

      lineX1 += GATE_WIDTH;
    }
    
    if (variable)
    {
      unsigned len = unsigned(variable->name().length()) * CHAR_WIDTH;
      if (int(len) < lineX2 - lineX1)
        len = lineX2 - lineX1;

      if(hasOption(SVGOption::NotInteractive))
        svgOut << svg::Line(lineX1, outy, lineX1 + len, outy, { LINK, variableCssClass(*variable) });
      else
        svgOut << svg::Line(lineX1, outy, lineX1 + len, outy, { LINK, gateCssClass(expression) });

      svgOut << svg::Text(lineX1 + CHAR_OFFSET, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, variable->name().c_str(), {});
    }
    else
    {
      svgOut << svg::Line(lineX1, outy, lineX2, outy, { LINK, gateCssClass(expression) });
      if (unary != plc::Term::Unary::None)
        svgOut << svg::Circle(lineX2 + INVERT_RADIUS, outy, INVERT_RADIUS, { INVERT, gateCssClass(expression) });

      if(hasOption(SVGOption::LinkLabels))
        svgOut << svg::Text(lineX2 - 3 * CHAR_WIDTH, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, gateCssClass(expression), {});
    }

    if (signalCrossing.find(expression.signalName()) != signalCrossing.end())
      inputPosition.emplace(expression.signalName(), COORD{ lineX1 + CROSSING_WIDTH, outy });

    expressionJsEquation(expression);

    return size;
  }

  unsigned crossingWidthUpToLevel(unsigned level)
  {
    unsigned crossingWidth = 0;
    for (int i = int(crossingsPerLevel.size()) - 1; i > int(level); i--)
      crossingWidth += crossingsPerLevel[i];

    return crossingWidth * CHAR_WIDTH;
  }

  void convertInput(const plc::Term& term, unsigned level, unsigned y, unsigned width)
  {
    const Variable& variable = plcAst.getVariable(term.identifier());

    unsigned x = XSTART;
    auto input = inputPosition.find(term.identifier());

    if (input == inputPosition.end() )
    {
      auto inputCount = inputCountMap.find(term.identifier());
      if (inputCount != inputCountMap.end() && inputCount->second > 1)
      {
        //std::cout << "crossing " << term.identifier() << ": " << crossingCount << std::endl;
        inputPosition.emplace(term.identifier(), COORD{ XSTART + textWidth + crossingCount * CROSSING_WIDTH, y });
        ++crossingCount;
      }

      std::ostringstream id;
      id << variableTypeIdentifier(variable.type()) << variable.index();
      svgOut << svg::Text(x, y + CHAR_OFFSET_Y, term.identifier(), { VARIABLE, variableCssClass(variable) }, id.str().c_str());
    }
    else
    {
      x = input->second.x;
      svgOut << svg::Circle(x, input->second.y, JOIN_RADIUS, { JOIN, variableCssClass(variable) })
        << svg::Line(x, input->second.y, x, y, { LINK, variableCssClass(variable) });

      input->second.y = y;
    }
    
    int lineX1 = crossingWidthUpToLevel(level) + width;

    if (term.unary() == plc::Term::Unary::None)
      svgOut << svg::Line(x, y, lineX1, y, { LINK, variableCssClass(variable) });
    else
      svgOut << svg::Line(x, y, lineX1 - 2 * INVERT_RADIUS, y, { LINK, variableCssClass(variable) })
      << svg::Circle(lineX1 - INVERT_RADIUS, y, INVERT_RADIUS, { INVERT, variableCssClass(variable) });

    if (hasOption(SVGOption::LinkLabels))
      svgOut << svg::Text(lineX1 - 3 * CHAR_WIDTH, y + CHAR_OFFSET_Y, variableCssClass(variable), {});
  }
  
  void expressionJsEquation(const plc::Expression& expression)
  {
    if (!hasOption(SVGOption::NotInteractive) || expression.op() != plc::Expression::Operator::Timer)
    {
      jsOut << "data[" << expression.jsTypeIndex() << "][" << expression.id() << "]= ";

      bool first = true;
      char opChar = (expression.op() == plc::Expression::Operator::And) ? '&' : '|';
      for (const plc::Term& t : expression.terms())
      {
        if (first)
          first = false;
        else
          jsOut << ' ' << opChar << ' ';

        if (t.unary() == plc::Term::Unary::Not)
          jsOut << '!';

        switch (t.type())
        {
        case plc::Term::Type::Identifier:
        {
          const Variable& v = plcAst.getVariable(t.identifier());
          jsOut << "data[" << static_cast<int>(v.type()) << "][" << v.index() << ']';
        }
        break;

        case plc::Term::Type::Expression:
          jsOut << "data[" << t.expression()->jsTypeIndex() << "][" << t.expression()->id() << ']';
          break;
        }
      }

      jsOut << ";" << std::endl;
    }
  }

  unsigned yposLast = 0;

  unsigned maxLevel = 0;
  unsigned crossingCount = 0;
  unsigned textWidth = 0;

  std::vector<unsigned> crossingsPerLevel;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

