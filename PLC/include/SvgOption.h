#ifndef _INCLUDE_SVG_OPTION_H_
#define _INCLUDE_SVG_OPTION_H_

/// <summary>
/// SVG Options
/// </summary>
enum struct SVGOption
{
  // Non Interactive, means no Clickable Inputs
  NotInteractive,

  // No Javascript at all, includes NonInteractive
  NoJavascript,

  // The Link lines will be labled
  LinkLabels
};

#endif // !_INCLUDE_SVG_OPTION_H_

