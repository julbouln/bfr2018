#!/bin/ruby

Dir.glob("defs/uni/*.xml").each do |f|
	nm = File.basename(f)
	system("xsltproc tools/defs_conv.xslt #{f} |xmllint -format - > defs/new/uni/#{nm}")
end


Dir.glob("defs/bui/*.xml").each do |f|
	nm = File.basename(f)
	system("xsltproc tools/defs_conv.xslt #{f} |xmllint -format - > defs/new/bui/#{nm}")
end
