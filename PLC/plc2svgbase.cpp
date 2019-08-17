#include "plc2svgbase.h"

const char *Plc2svgBase::SVG_HEADER = R"~~~(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg" width="1000" height="1000">
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
</style>)~~~";

const char *Plc2svgBase::SVG_FUNCTIONS_JS_START = R"~~~(
<script type="text/javascript"><![CDATA[
function SVGData(inputs,outputs,timer,intermediates,flags,logicfn)
{
  this.data = [ Array(inputs).fill(false), Array(outputs).fill(false),
    Array(timer).fill(false), Array(flags).fill(false), Array(intermediates).fill(false) ];
  this.prefix = [ 'i', 'o', 'm', 'f', 'g' ];
  var that= this;
  
  function updateElement(name, value)
  {
    document.querySelectorAll('.' + name).forEach(function(obj)
    {
      if (value)
        obj.classList.add('on');
      else
        obj.classList.remove('on');
    });
  }
  this.update = function()
  {
    logicfn(that.data);
    
    that.prefix.forEach(function(p, i) 
    { 
      that.data[i].forEach(function(v, j){ updateElement(p + j, v); });
    });
  }
  this.setValue = function(type, idx, value)
  {
    that.data[type][idx]= value;    
    that.update();
    
    return that.data[type][idx];
  }
  this.toggleValue = function(type, idx)
  {
    that.data[type][idx]= !that.data[type][idx];    
    that.update();
    
    return that.data[type][idx];
  }
  this.toggleById = function(id)
  {
    let type= that.prefix.indexOf( id.charAt(0) );
    if( type >= 0 )
      that.toggleValue(type, Number.parseInt(id.substring(1)));
  }
  this.setText = function(name, text)
  {
    var t= document.getElementById('t' + name);
    if(t)
      t.textContent= text;
  }
}
document.addEventListener("DOMContentLoaded", function ()
{
)~~~";

const char *Plc2svgBase::SVG_FUNCTIONS_JS_END = R"~~~(
});]]></script>)~~~";

const char *Plc2svgBase::SVG_JS_FOOTER = R"~~~(
</defs>
)~~~";
