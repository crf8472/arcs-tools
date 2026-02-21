<?xml version="1.0" encoding="UTF-8"?>

<!-- Removes inline namespace names from class*.xml files.                  -->

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


<!-- Remove a specified namespace name -->
<!-- This will fail if 'full_name' does not contain 'ns_name' -->


<xsl:template name="remove_ns_name">
	<xsl:param name="full_name"/>
	<xsl:param name="ns_name"/>

	<xsl:value-of select="concat(
			substring-before($full_name, $ns_name),
			substring-after ($full_name, $ns_name))"/>
</xsl:template>


<!-- Process all elements containing namespace 'v_1_0_0' -->


<xsl:template match="compoundname[contains(text(),'::v_1_0_0')]">

	<xsl:element name="compoundname">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:element>

</xsl:template>

<xsl:template match="definition[contains(text(),'::v_1_0_0')]">

	<xsl:element name="definition">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:element>

</xsl:template>

<xsl:template match="qualifiedname[contains(text(),'::v_1_0_0')]">

	<xsl:element name="qualifiedname">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:element>

</xsl:template>

<xsl:template match="scope[contains(text(),'::v_1_0_0')]">

	<xsl:element name="scope">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:element>

</xsl:template>

<xsl:template match="label[contains(text(),'::v_1_0_0')]">

	<xsl:element name="label">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:element>

</xsl:template>

<!-- -->

<xsl:template match="innerclass[contains(text(),'::v_1_0_0')]">

	<xsl:copy>
		<xsl:apply-templates select="@*"/> <!-- copy element + attrs -->
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:copy>

</xsl:template>

<xsl:template match="innernamespace[contains(text(),'::v_1_0_0')]">

	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:copy>

</xsl:template>

<xsl:template match="basecompoundref[contains(text(),'::v_1_0_0')]">

	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:copy>

</xsl:template>

<xsl:template match="derivedcompoundref[contains(text(),'::v_1_0_0')]">

	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="text()"/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:copy>

</xsl:template>

<xsl:template match="@ambiguityscope[contains(.,'::v_1_0_0')]">

	<xsl:attribute name="ambiguityscope">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="."/>
			<xsl:with-param name="ns_name"   select="'::v_1_0_0'"/>
		</xsl:call-template>
	</xsl:attribute>

</xsl:template>

<xsl:template match="@refid[contains(.,'_1_1v__1__0__0')]">

	<xsl:attribute name="refid">
		<xsl:call-template name="remove_ns_name">
			<xsl:with-param name="full_name" select="."/>
			<xsl:with-param name="ns_name"   select="'_1_1v__1__0__0'"/>
		</xsl:call-template>
	</xsl:attribute>

</xsl:template>

</xsl:transform>

