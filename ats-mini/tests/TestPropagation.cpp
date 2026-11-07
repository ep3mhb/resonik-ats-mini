#include "../Propagation.h"
#include <assert.h>
#include <stdio.h>

static const char *sample =
"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<solar>\n<solardata>\n"
"<source url=\"http://www.hamqsl.com/solar.html\">N0NBH</source>\n"
"<updated> 07 Nov 2026 0745 GMT</updated>\n<solarflux>145</solarflux>\n<aindex> 8</aindex>\n"
"<kindex> 2</kindex>\n<kindexnt>No Report</kindexnt>\n<xray>B5.6</xray>\n<sunspots>120</sunspots>\n"
"<heliumline>143.2</heliumline>\n<geomagfield>QUIET</geomagfield>\n"
"<calculatedconditions>\n"
"<band name=\"80m-40m\" time=\"day\">Fair</band>\n<band name=\"30m-20m\" time=\"day\">Good</band>\n"
"<band name=\"17m-15m\" time=\"day\">Good</band>\n<band name=\"12m-10m\" time=\"day\">Fair</band>\n"
"<band name=\"80m-40m\" time=\"night\">Good</band>\n<band name=\"30m-20m\" time=\"night\">Good</band>\n"
"<band name=\"17m-15m\" time=\"night\">Fair</band>\n<band name=\"12m-10m\" time=\"night\">Poor</band>\n"
"</calculatedconditions>\n<calculatedvhfconditions>\n"
"<phenomenon name=\"vhf-aurora\" location=\"northern_hemi\">Band Closed</phenomenon>\n"
"</calculatedvhfconditions>\n</solardata>\n</solar>\n";

int main()
{
  PropData d;
  assert(propParse(sample, &d));
  assert(d.valid && d.sfi == 145 && d.ssn == 120 && d.aIndex == 8 && d.kIndex == 2);
  assert(!strcmp(d.xray, "B5.6"));
  assert(!strcmp(d.updated, "07 Nov 2026 0745 GMT"));
  assert(!strcmp(d.geomag, "QUIET"));
  assert(d.day[0] == PROP_FAIR && d.day[1] == PROP_GOOD && d.day[3] == PROP_FAIR);
  assert(d.night[0] == PROP_GOOD && d.night[2] == PROP_FAIR && d.night[3] == PROP_POOR);
  assert(!propParse("<html>error</html>", &d) && !d.valid);
  printf("TestPropagation: all tests passed\n");
  return 0;
}
