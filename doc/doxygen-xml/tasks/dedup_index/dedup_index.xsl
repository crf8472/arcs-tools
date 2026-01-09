<?xml version="1.0" encoding="UTF-8"?>

<!-- Removes duplicate <compound>-entries in the index.xml file.            -->
<!-- This fixes the duplicate classnames in doxygen's 'Classes' index when  -->
<!-- using inline namespaces in C++.                                        -->

<!-- Each compound with a given refid occurs twice: once for arcstk::Class  -->
<!-- and a second time for inline namespace as arcstk::v_1_0_0::Class while -->
<!-- the content of these compounds is identical. This script respects the  -->
<!-- first occurrence for each refid and removes all following duplicates.  -->
<!-- Thus, the version without the inline namespace is kept.                -->

<!-- Required for doxygen >= 1.8.16.                                        -->

<xsl:transform
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xsi:noNamespaceSchemaLocation="index.xsd">

<xsl:output method="xml" standalone="no" indent="yes" />
<xsl:strip-space elements="*"/>

<xsl:key name="keyCompoundById" match="compound" use="@refid"/>

<!-- Just copy everything -->

<xsl:template match="node()|@*">
	<xsl:copy>
		<xsl:apply-templates select="node()|@*"/>
	</xsl:copy>
</xsl:template>

<!-- Remove any compound with the same refid, keep the last occurrence. -->
<!-- This is called "Muenchian grouping". -->

<xsl:template match="compound[
	not(generate-id() = generate-id(key('keyCompoundById', @refid)[1])) ]">
</xsl:template>

</xsl:transform>

