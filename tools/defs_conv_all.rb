#!/bin/ruby

# helper script to convert old defs to new defs format

class DefsConverter
	def self.convert_units
		Dir.glob("defs/uni/*.xml").each do |f|
			nm = File.basename(f)
			system("xsltproc tools/defs_conv.xslt #{f} |xmllint -format - > defs/new/uni/#{nm}")
		end
	end

	def self.convert_buildings
		Dir.glob("defs/bui/*.xml").each do |f|
			nm = File.basename(f)
			system("xsltproc tools/defs_conv.xslt #{f} |xmllint -format - > defs/new/bui/#{nm}")
		end
	end

	def self.convert_resources(name)
		system("xsltproc tools/defs_conv.xslt defs/res/#{name}.xml | xmllint -format - > defs/new/res/#{name}_full.xml")
		system("xml_split -n 1 defs/new/res/#{name}_full.xml")
		Dir.glob("defs/new/res/#{name}_full-*.xml").each do |f|
			num=f.gsub(/.*\-(\d+)\.xml/,'\1').to_i
			system("mv #{f} defs/new/res/#{name}#{num}.xml")
		end
		system("rm defs/new/res/#{name}0.xml")
		system("rm defs/new/res/#{name}_full.xml")
	end


end



DefsConverter.convert_resources("nature")
DefsConverter.convert_resources("pollution")