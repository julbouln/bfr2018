<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<!-- unit -->
	<xsl:template match="char">
		<entity class="unit">
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:attribute name="team">
				<xsl:value-of select="@team"/>
			</xsl:attribute>
			<xsl:apply-templates select="file">
				<xsl:with-param name="mode">with_directions</xsl:with-param>
			</xsl:apply-templates>
			<xsl:apply-templates select="icon"/>
			<xsl:apply-templates select="face"/>
			<xsl:apply-templates select="spe"/>
			<tile directions="8">
				<xsl:apply-templates select="pixel_size"/>
				<xsl:apply-templates select="case_size"/>
				<xsl:apply-templates select="decal_value"/>
				<xsl:apply-templates select="states"/>
			</tile>
			<game_object>
				<name>
					<xsl:attribute name="value">
						<xsl:value-of select="@name"/>
					</xsl:attribute>
				</name>
				<team>
					<xsl:attribute name="value">
						<xsl:value-of select="@team"/>
					</xsl:attribute>
				</team>
				<view>
					<xsl:attribute name="value">
						<xsl:value-of select="view/@dist"/>
					</xsl:attribute>
				</view>
				<xsl:copy-of select="life"/>
				<effects>
					<xsl:apply-templates select="projectile"/>
				</effects>
			</game_object>
			<unit>
				<xsl:copy-of select="speed"/>
				<xsl:copy-of select="attack1"/>
				<xsl:copy-of select="attack2"/>
				<sound_actions>
					<xsl:apply-templates select="select_sounds"/>
					<xsl:for-each select="states/state">
						<xsl:call-template name="states_sounds"/>
					</xsl:for-each>
				</sound_actions>
				<xsl:choose>
					<xsl:when test="count(projectile/sounds/sound) &gt; 0">
						<sound_attack>
							<xsl:attribute name="name">
								<xsl:value-of select="projectile/@name"/>
							</xsl:attribute>
						</sound_attack>
					</xsl:when>
					<xsl:otherwise/>
				</xsl:choose>
			</unit>
		</entity>
	</xsl:template>
	<!-- building -->
	<xsl:template match="building">
		<entity class="building">
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:attribute name="team">
				<xsl:value-of select="@team"/>
			</xsl:attribute>
			<xsl:apply-templates select="file">
				<xsl:with-param name="mode">with_white_mask</xsl:with-param>
			</xsl:apply-templates>
			<xsl:apply-templates select="icon"/>
			<xsl:apply-templates select="face"/>
			<xsl:apply-templates select="spe"/>
			<tile>
				<xsl:apply-templates select="pixel_size"/>
				<xsl:apply-templates select="case_size"/>
				<xsl:apply-templates select="decal_value"/>
				<xsl:apply-templates select="states"/>
			</tile>
			<game_object>
				<name>
					<xsl:attribute name="value">
						<xsl:value-of select="@name"/>
					</xsl:attribute>
				</name>
				<team>
					<xsl:attribute name="value">
						<xsl:value-of select="@team"/>
					</xsl:attribute>
				</team>
				<view>
					<xsl:attribute name="value">
						<xsl:value-of select="view/@dist"/>
					</xsl:attribute>
				</view>
				<xsl:copy-of select="life"/>
				<effects>
					<effect name="explosion" ref="explosion"/>
				</effects>
			</game_object>
			<building>
				<xsl:copy-of select="build_time"/>
			</building>
		</entity>
	</xsl:template>
	<!-- resource -->
	<xsl:template match="resources">
		<entities>
			<xsl:variable name="name" select="@name"/>
			<xsl:variable name="team" select="@team"/>
			<xsl:for-each select="*">
				<entity class="resource" name="resource">
					<xsl:attribute name="name">
						<xsl:value-of select="$name"/>
						<xsl:value-of select="position()"/>
					</xsl:attribute>
					<xsl:attribute name="group">
						<xsl:value-of select="$name"/>
					</xsl:attribute>
					<xsl:attribute name="team">
						<xsl:choose>
							<xsl:when test="$name = 'nature'">rebel</xsl:when>
							<xsl:otherwise>neonaz</xsl:otherwise>
						</xsl:choose>
					</xsl:attribute>
					<xsl:apply-templates select="file">
						<xsl:with-param name="mode">with_white_mask</xsl:with-param>
					</xsl:apply-templates>
					<tile>
						<xsl:apply-templates select="pixel_size"/>
						<xsl:apply-templates select="case_size"/>
						<xsl:choose>
							<xsl:when test="count(decal_value) &gt; 0">
								<offset>
									<xsl:attribute name="x">
										<xsl:value-of select="decal_value/@w"/>
									</xsl:attribute>
									<xsl:attribute name="y">
										<xsl:value-of select="decal_value/@h"/>
									</xsl:attribute>
								</offset>
							</xsl:when>
						</xsl:choose>
						<!--
						<animations>
							<animation name="idle">
								<xsl:apply-templates select="frames"/>
								<xsl:apply-templates select="refresh"/>
							</animation>
						</animations>
					-->
					</tile>
					<resource>
						<level value="1"/>
					</resource>
				</entity>
			</xsl:for-each>
		</entities>
	</xsl:template>
	<xsl:template match="file">
		<xsl:param name="mode"/>
		<texture mode="with_directions">
			<xsl:attribute name="mode">
				<xsl:value-of select="$mode"/>
			</xsl:attribute>
			<xsl:attribute name="path">
				<xsl:value-of select="@path"/>
			</xsl:attribute>
		</texture>
	</xsl:template>
	<xsl:template match="icon">
		<texture mode="build_button" name="icon">
			<xsl:attribute name="path">
				<xsl:value-of select="@path"/>
			</xsl:attribute>
		</texture>
	</xsl:template>
	<xsl:template match="face">
		<texture name="face">
			<xsl:attribute name="path">
				<xsl:value-of select="@path"/>
			</xsl:attribute>
		</texture>
	</xsl:template>
	<xsl:template match="spe">
		<texture name="spe">
			<xsl:attribute name="path">
				<xsl:value-of select="@path"/>
			</xsl:attribute>
		</texture>
	</xsl:template>
	<xsl:template match="pixel_size">
		<psize>
			<xsl:attribute name="x">
				<xsl:value-of select="@w"/>
			</xsl:attribute>
			<xsl:attribute name="y">
				<xsl:value-of select="@h"/>
			</xsl:attribute>
		</psize>
	</xsl:template>
	<xsl:template match="case_size">
		<size>
			<xsl:attribute name="x">
				<xsl:value-of select="@w"/>
			</xsl:attribute>
			<xsl:attribute name="y">
				<xsl:value-of select="@h"/>
			</xsl:attribute>
		</size>
	</xsl:template>
	<xsl:template match="decal_value">
		<offset>
			<xsl:attribute name="x">
				<xsl:value-of select="@x"/>
			</xsl:attribute>
			<xsl:attribute name="y">
				<xsl:value-of select="@y"/>
			</xsl:attribute>
		</offset>
	</xsl:template>
	<xsl:template match="states">
		<animations>
			<xsl:apply-templates select="state"/>
		</animations>
	</xsl:template>
	<xsl:template match="state">
		<animation>
			<xsl:attribute name="name">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
			<xsl:apply-templates select="frames"/>
			<xsl:apply-templates select="refresh"/>
		</animation>
	</xsl:template>
	<xsl:template match="frames">
		<frames>
			<xsl:copy-of select="*"/>
		</frames>
	</xsl:template>
	<xsl:template match="refresh">
		<duration>
			<xsl:attribute name="value">
				<xsl:value-of select="@value * 100"/>
			</xsl:attribute>
		</duration>
	</xsl:template>
	<xsl:template name="states_sounds">
		<xsl:choose>
			<xsl:when test="count(sounds/sound) &gt; 0">
				<sound_action>
					<xsl:attribute name="name">
						<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:for-each select="sounds/*">
						<xsl:call-template name="sound"/>
					</xsl:for-each>
				</sound_action>
			</xsl:when>
			<xsl:otherwise/>
		</xsl:choose>
	</xsl:template>
	<xsl:template match="select_sounds">
		<xsl:choose>
			<xsl:when test="count(sound) &gt; 0">
				<sound_action name="select">
					<xsl:for-each select="*">
						<xsl:call-template name="sound"/>
					</xsl:for-each>
				</sound_action>
			</xsl:when>
			<xsl:otherwise/>
		</xsl:choose>
	</xsl:template>
	<xsl:template name="sound">
		<sound_buffer>
			<xsl:attribute name="path">
				<xsl:value-of select="@path"/>
			</xsl:attribute>
		</sound_buffer>
	</xsl:template>
	<xsl:template match="projectile">
		<effect name="projectile">
			<xsl:attribute name="ref">
				<xsl:value-of select="@name"/>
			</xsl:attribute>
		</effect>
	</xsl:template>
</xsl:stylesheet>