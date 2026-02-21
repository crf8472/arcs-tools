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

<!-- Default: just copy everything -->

<xsl:template match="node()|@*">
	<xsl:copy>
		<xsl:apply-templates select="node()|@*"/>
	</xsl:copy>
</xsl:template>

<!-- Remove a specified namespace name from a compound/name.          -->
<!-- This will fail if 'full_name' does not contain 'ns_name' or '::' -->

<xsl:template name="remove-ns_name">
	<xsl:param name="full_name"/>
	<xsl:param name="ns_name"/>

	<!-- Inject specified namespace name -->
	<xsl:value-of select="concat(
			substring-before($full_name, $ns_name),
			substring-after ($full_name, $ns_name))"/>
</xsl:template>

<!-- Remove any compound with the same refid, keep the last occurrence. -->
<!-- This is called "Muenchian grouping". -->

<xsl:template match="compound/name[contains(text(),'::v_1_0_0')]">

		<xsl:element name="name">
			<xsl:call-template name="remove-ns_name">
				<xsl:with-param name="full_name" select="text()"/>
				<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
			</xsl:call-template>
		</xsl:element>

</xsl:template>

<xsl:template match="compound[@refid='namespacearcsapp_1_1v__1__0__0' and @kind='namespace']">
	<!-- remove -->
</xsl:template>

</xsl:transform>

