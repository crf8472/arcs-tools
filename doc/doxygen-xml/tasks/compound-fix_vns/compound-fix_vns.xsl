<?xml version="1.0" encoding="UTF-8"?>

<!-- Insert missing inline namespace names in <compoundname>s.              -->

<!-- Required for doxygen >= 1.8.16.                                        -->

<xsl:transform
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xsi:noNamespaceSchemaLocation="index.xsd">

<xsl:output method="xml" standalone="no" indent="yes" />
<xsl:strip-space elements="*"/>


<!-- Default: Just copy everything -->

<xsl:template match="node()|@*">
	<xsl:copy>
		<xsl:apply-templates select="node()|@*"/>
	</xsl:copy>
</xsl:template>


<!-- Insert a specified namespace name after the first namespace. -->
<!-- This will fail if 'full_name' does not contain 'ns_name' or '::' -->

<xsl:template name="insert_ns_name">
	<xsl:param name="full_name"/>
	<xsl:param name="ns_name"/>

	<!-- Inject specified namespace name -->
	<xsl:value-of select="concat(
			substring-before($full_name, '::'),
			'::', $ns_name, '::',
			substring-after ($full_name, '::'))"/>
</xsl:template>


<!-- Match all compoundnames that have names with more than one       -->
<!-- namespace (thus leaving out the top-level ns) but do not already -->
<!-- contain the inline namespace '::v_1_0_0'.                        -->

<xsl:template match="compoundname[contains(text(),'::') and
	not(contains(text(),'::v_1_0_0'))]">

		<xsl:element name="compoundname">
			<xsl:call-template name="insert_ns_name">
				<xsl:with-param name="full_name" select="text()"/>
				<xsl:with-param name="ns_name"   select="'v_1_0_0'"/>
			</xsl:call-template>
		</xsl:element>

</xsl:template>

</xsl:transform>

