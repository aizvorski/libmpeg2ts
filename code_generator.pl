#!/usr/bin/perl

# libmpeg2ts, a MPEG2 transport stream reading/writing library
# 
# Copyright (C) 2012 Alexander Izvorski <aizvorski@gmail.com>
# 
# This file is part of libmpeg2ts.
# 
# libmpeg2ts is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# libmpeg2ts is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with libmpeg2ts.  If not, see <http://www.gnu.org/licenses/>.

use XML::LibXML;
use IO::File;
use IO::All;

my $parser = XML::LibXML->new();
my $doc    = $parser->parse_file('mpeg2ts.xml');

$indent = 0;
$output_fh = IO::File->new('mpeg2ts.h', 'w');

$output_fh->print(io('mpeg2ts.h_prefix')->all);
&process_node($doc, "structs");
&process_node($doc, "defs");
$output_fh->print(io('mpeg2ts.h_suffix')->all);

$output_fh = IO::File->new('mpeg2ts.c', 'w');

$output_fh->print(io('mpeg2ts.c_prefix')->all);
&process_node($doc, "read_func");
&process_node($doc, "write_func");
&process_node($doc, "len_func");

sub output { $output_fh->print ("    " x $indent); $output_fh->print(@_); }


sub process_node
{
    my ($node, $gen) = @_;

    my ($name, $type, $length, $value);
    if ($node->nodeName !~ m!\#!) { ($name, $type, $length, $value) = ($node->getAttribute("name"), $node->getAttribute("type"), $node->getAttribute("length"), $node->getAttribute("value")); }

    if ($node->nodeName() eq 'structure')
    {
        $indent = 0;

        if ($gen eq "write_func")   { &output("int ${name}_write(${name}_t* p, bs_t* bs) {\n"); }
        elsif ($gen eq "read_func") { &output("int ${name}_read(${name}_t* p, bs_t* bs) {\n"); }
        elsif ($gen eq "len_func") 
        {
            &output("int ${name}_len(${name}_t* p) {\n");

            $indent++;
            &output("int len;\n");

            # find all child nodes which are lengths, in deepest-first order
            my @nodes = $node->findnodes(".//length");
            @nodes = reverse @nodes;

            # for each length node, add up the lengths of all next siblings as subtrees, 
            # except that any length nodes are just added directly and their next siblings skipped
            foreach my $length_node (@nodes)
            {
                &output("len = 0;\n");
                my $next_node = $length_node;
                while ($next_node = $next_node->nextSibling)
                {
                    &process_node($next_node, "len_func");
                }
                &output("if (p->".$length_node->getAttribute("field")." < len /8) { p->".$length_node->getAttribute("field")." = len /8; };\n\n");
            }

            &output("len = 0;\n");
            $indent--;
        }
        elsif ($gen eq "structs")
        {
            &output("typedef struct {\n");
            %fields_hash = ();
        }
        elsif ($gen eq "defs")
        {
            &output("int ${name}_write(${name}_t* p, bs_t* bs);\n");
            &output("int ${name}_read(${name}_t* p, bs_t* bs);\n");
            &output("int ${name}_len(${name}_t* p);\n");
        }
    }
    elsif ($node->nodeName() eq 'if' && $gen ne "structs" && $gen ne "defs")
    {
        if ($node->hasAttribute("equals"))
        {
            &output("if (p->".$node->getAttribute("field")." == ".$node->getAttribute("equals").") {\n"); 
        }
        elsif ($node->hasAttribute("equals_any"))
        {
            my $field = $node->getAttribute("field");
            &output("if (".join(" || ", map { "p->${field} == $_" } split(/,/, $node->getAttribute("equals_any")) ) .") {\n"); 
        }
        elsif ($node->hasAttribute("not_equals_any"))
        {
            my $field = $node->getAttribute("field");
            &output("if (".join(" && ", map { "p->${field} != $_" } split(/,/, $node->getAttribute("not_equals_any")) ) .") {\n"); 
        }
        elsif ($node->hasAttribute("greater_than"))
        {
            &output("if (p->".$node->getAttribute("field")." > ".$node->getAttribute("greater_than").") {\n"); 
        }
        else
        {
            # TODO
            &output("if () { // FIXME\n");
        }
    }
    elsif ($node->nodeName() eq 'field')
    {
        if ($node->hasAttribute("value")) # constants
        {
            if ($node->getAttribute("type") eq 'bytes' && $node->getAttribute("byte_length") eq 'all')
            {
                if ($gen eq "write_func")   { &output("bs_fill_bytes_all(bs, ${value});\n"); }
                elsif ($gen eq "read_func") { &output("bs_skip_bytes_all(bs);\n"); }
                elsif ($gen eq "len_func")  { } # this is zero-length for the purpose of calculating minimum lengths
                elsif ($gen eq "structs")   { }
            }
            elsif ($node->getAttribute("type") eq 'bytes')
            {
                if ($gen eq "write_func")   { &output("bs_fill_bytes(bs, ${value}, p->".$node->getAttribute("byte_length").");\n"); }
                elsif ($gen eq "read_func") { &output("bs_skip_bytes(bs, p->".$node->getAttribute("byte_length").");\n"); }
                elsif ($gen eq "len_func")  { &output("len += ".$node->getAttribute("byte_length")." *8;\n"); }
                elsif ($gen eq "structs")   { }
            }
            elsif ($node->getAttribute("type") =~ m!^(bslbf|uimsbf|tcimsbf)$!)
            {
                if ($gen eq "write_func")   { &output("bs_write_${type}(bs, ${value}, ${length});\n"); }
                elsif ($gen eq "read_func") { &output("bs_skip_${type}(bs, ${length});\n"); }
                elsif ($gen eq "len_func")  { &output("len += ${length};\n"); }
                elsif ($gen eq "structs")   { }
            }
            else
            {
                &output("// ${type}(${length}) ${name} // FIXME\n");
            }
        }
        else
        {
            if ($gen eq "structs" && $fields_hash{ ${name} }) { return; }

            if ($node->getAttribute("type") eq 'bytes' && $node->getAttribute("byte_length") eq 'all')
            {
                if ($gen eq "write_func")   { &output("bs_write_bytes(bs, p->${name}, p->${name}_len);\n"); }
                elsif ($gen eq "read_func") { &output("p->${name}_len = bs_read_bytes_all(bs, p->${name});\n"); }
                elsif ($gen eq "len_func")  { &output("len += p->${name}_len *8;\n"); }
                elsif ($gen eq "structs")   { &output("uint8_t* ${name};\n"); &output("int ${name}_len;\n"); }
            }
            elsif ($node->getAttribute("type") eq 'bytes')
            {
                if ($gen eq "write_func")   { &output("bs_write_bytes(bs, p->${name}, p->${name}_len);\n"); }
                elsif ($gen eq "read_func") { &output("p->${name}_len = bs_read_bytes(bs, p->${name}, p->".$node->getAttribute("byte_length").");\n"); }
                elsif ($gen eq "len_func")  { &output("len += p->${name}_len *8;\n"); &output("p->".$node->getAttribute("byte_length")." = p->${name}_len;\n"); }
                elsif ($gen eq "structs")   { &output("uint8_t* ${name};\n"); &output("int ${name}_len;\n"); }
            }
            elsif ($node->getAttribute("type") eq 'structure')
            {
                if ($gen eq "write_func")   { &output("".$node->getAttribute("structure_type")."_write(p->${name}, bs);\n"); }
                elsif ($gen eq "read_func") { &output("".$node->getAttribute("structure_type")."_read(p->${name}, bs);\n"); }
                elsif ($gen eq "len_func")  { &output("len += ".$node->getAttribute("structure_type")."_len(p->${name});\n"); }
                elsif ($gen eq "structs")   { &output("".$node->getAttribute("structure_type")."_t* ${name};\n"); }
            }
            elsif ($node->getAttribute("type") =~ m!^(bslbf|uimsbf|tcimsbf|timestamp_with_markers)$!)
            {
                if ($gen eq "write_func")   { &output("bs_write_${type}(bs, p->${name}, ${length});\n"); }
                elsif ($gen eq "read_func") { &output("p->${name} = bs_read_${type}(bs, ${length});\n"); }
                elsif ($gen eq "len_func")  { &output("len += ${length};\n"); }
                elsif ($gen eq "structs")   { &output("int ${name};\n"); }
            }
            else
            {
                &output("// ${type}(${length}) ${name} // FIXME\n");
            }

            if ($gen eq "structs") { $fields_hash{ ${name} } = 1; }
        }
    }
    elsif ($node->nodeName() eq 'constant')
    {
        if ($gen eq "structs")   { &output("#define ${name} ${value}\n"); }
    }
    elsif ($node->nodeName() eq 'length' && $gen eq 'len_func')
    {
        &output("len += p->".$node->getAttribute("field")." *8;\n");
    }

    $indent++;
    foreach my $child ($node->childNodes())
    {
        &process_node($child, $gen);
        if ($gen eq 'len_func' && $child->nodeName() eq 'length')
        {
            last;
        }
    }
    $indent--;

    if ($node->nodeName() eq 'structure')
    {
        if ($gen eq "write_func")   { &output("};\n\n"); }
        elsif ($gen eq "read_func") { &output("};\n\n"); }
        elsif ($gen eq "len_func") 
        {
            $indent++;
            &output("return len /8;\n");
            $indent--;
            &output("};\n\n");
        }
        elsif ($gen eq "structs")   { &output("} ${name}_t;\n\n"); }
    }
    elsif ($node->nodeName() eq 'if' && $gen ne "structs" && $gen ne "defs")
    {
        &output("};\n");
    }
}


