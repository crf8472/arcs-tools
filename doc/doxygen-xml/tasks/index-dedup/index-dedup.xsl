<?xml version="1.0" encoding="UTF-8"?>

<!-- Removes duplicate <compound>-entries in the index.xml file.            -->
<!-- This fixes the duplicate classnames in doxygen's 'Classes' index when  -->
<!-- using inline namespaces in C++.                                        -->

<!-- Each compound with a given refid occurs twice: once for arcstk::Class  -->
<!-- and a second time for inline namespace as arcstk::v_1_0_0::Class while -->
<!-- the content of these compounds is identical.                           -->
<!-- The version containing the inline namespace is kept.                   -->

<!-- Required for doxygen >= 1.8.16.                                        -->

<xsl:transform
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xsi:noNamespaceSchemaLocation="index.xsd">

<xsl:output method="xml" standalone="no" indent="yes" />
<xsl:strip-space elements="*"/>

<xsl:key name="keyCompoundById" match="compound" use="@refid"/>


<!-- Default: Just copy everything -->

<xsl:template match="node()|@*">
	<xsl:copy>
		<xsl:apply-templates select="node()|@*"/>
	</xsl:copy>
</xsl:template>


<!-- Add version namespace just everywhere. -->
<!-- Thus we will not have to care about accessing a preceding sibling. -->

<xsl:template match="compound/name[not(contains(text(), '::v_1_0_0'))
	and contains(text(), '::')]">

	<xsl:element name="name">
		<xsl:value-of select="concat(
			substring-before(text(), '::'),
			'::v_1_0_0::',
			substring-after(text(), '::'))"/>
	</xsl:element>

</xsl:template>


<!-- Remove each compound with a refid that previously occurred by -->
<!-- "Muenchian grouping".                                         -->

<xsl:template match="compound[
	not(generate-id() = generate-id(key('keyCompoundById', @refid)[1]))]">
	<!-- remove current -->
</xsl:template>

</xsl:transform>

