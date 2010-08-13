<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
<html>
  <body>
  <title>Abstract Data Communication Library Records</title>
  <style type="text/css">
    h1 {
	font-size: 36px;
	color: #005db6;
	margin-top: 10px;
	margin-bottom: 13px;
    }
    h2 {
	font-size: 16px;
	color: #000000;
	margin-top: 0px;
	margin-bottom: 3px;
    }
  </style>
    <h1 align ="center">ADCL Records</h1>
    <table align ="center" border="1">
    <tr bgcolor="#9AC0CD">
      <th align="center">Topology information</th>
      <th align="center">Vector information</th>
      <th align="center">Attribute information</th>
      <th align="center">Function information</th>
    </tr>
    <xsl:for-each select="ADCL/RECORD">
    <tr bgcolor="#F5F5DC">
      <td>
        <table> 
          <tr>
            <td><b>Topology dimensions :</b></td>
            <td> <xsl:value-of select="TOPO/NDIM"/></td>
          </tr>
          <tr>
            <td><b>Topology perodicity :</b></td>
            <td><xsl:value-of select="TOPO/PERIOD"/></td>
          </tr>
        </table>
      </td>
      <td>
        <table> 
          <tr>
            <td><b>Num of vector dimensions :</b></td>
            <td> <xsl:value-of select="VECT/NDIM"/></td>
          </tr>
          <tr>
            <td><b>Vector dimensions :</b></td>
            <td><xsl:value-of select="VECT/DIMS"/></td>
          </tr>
          <tr>
            <td><b>NC :</b></td>
            <td><xsl:value-of select="VECT/NC"/></td>
          </tr>
          <tr>
            <td><b>Halo cells width :</b></td>
            <td><xsl:value-of select="VECT/HWIDTH"/></td>
          </tr>
          <tr>
            <td><b>Communication type :</b></td>
            <td><xsl:value-of select="VECT/COMTYPE"/></td>
          </tr>
        </table>
      </td>
      <td>
        <table> 
          <tr>
            <td><b>Number of attributes :</b></td>
            <td> <xsl:value-of select="ATTR/NUM"/></td>
          </tr>
          <tr>
            <td><b>Attribute values :</b></td>
            <td><xsl:value-of select="ATTR/ATTRVALS"/></td>
          </tr>
        </table>
      </td>
      <td>
        <table> 
          <tr>
            <td><b>Function set :</b></td>
            <td> <xsl:value-of select="FUNC/FNCTSET"/></td>
          </tr>
          <tr>
            <td><b>Winner Function :</b></td>
            <td><xsl:value-of select="FUNC/WINNER"/></td>
          </tr>
        </table>
      </td>
    </tr>
    </xsl:for-each>
    </table>
  </body>
</html>
</xsl:template>
</xsl:stylesheet>