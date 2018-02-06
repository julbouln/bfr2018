<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<!-- unit -->
	<xsl:template match="entity/tile/animations">
		<spritesheets>
			<xsl:apply-templates select="animation"/>
		</spritesheets>
	</xsl:template>
	<xsl:template match="animation">
		<views count="8" type="animated">
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<view>
				<xsl:copy-of select="frames"/>
				<xsl:copy-of select="duration"/>
			</view>
		</views>
	</xsl:template>
</xsl:stylesheet>