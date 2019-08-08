#include "plc2svg.h"

const char *Plc2svg::SVG_HEADER = R"~~~(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg" width="800" height="800">
<defs>
<style type="text/css">
<![CDATA[
rect.box { stroke:#000; fill:none; }
circle.invert { stroke:#000; fill:none; }
circle.join { stroke:#000; fill:#000; }
line.link { stroke:#000; stroke-width:1px; }
line.on { stroke:#0f0; }
line.test { stroke:#f00; }
]]>
</style>
<script type="text/javascript"><![CDATA[
document.addEventListener("DOMContentLoaded", function ()
{
var i=new Array()~~~";


const char *Plc2svg::SVG_FUNCTIONS = R"~~~(function updateElement(name, value)
{
  document.querySelectorAll('.' + name).forEach(function(obj)
  {
    if (value)
      obj.classList.add('on');
    else
      obj.classList.remove('on');
  });
}
function logic()
{
  g[0] = !i[0] | !i[1];
  i.forEach(function(v, i) { updateElement('i' + i, v); });
  g.forEach(function(v, i) { updateElement('g' + i, v); });
}
function toggleInput(event)
{
  let index = event.target.id;
  i[index] = !i[index];
  logic();
  console.log('i[' + index + ']=' + i[index]);
}
)~~~";

const char *Plc2svg::SVG_FOOTER = R"~~~(});
]]></script>
</defs>
)~~~";