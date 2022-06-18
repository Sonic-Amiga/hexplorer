SWITCHES = -w -O3 -march=pentium -Isrecord-1.16/include

hexplo.exe : hexplorer.o container.o undo.o position.o fonts.o macro.o filetype.o mclip.o tool.o hex_common.o rsrc.o srec_win_quit.o           libdis.o i386_invariant.o i386.o               MessageDigest.o sha.o md5.o ripemd.o           mediaaccess.o            record.o input.o output.o input_file.o output_file.o        input_ascii_hex.o input_atmel_generic.o input_dec_binary.o input_emon52.o input_fastload.o input_four_packed_code.o input_intel.o input_mos_tech.o input_srecord.o input_os65v.o input_signetics.o input_spasm.o input_tektronix.o input_tektronix_extended.o input_ti_tagged.o input_wilson.o input_cosmac.o input_fairchild.o input_formatted_binary.o input_needham.o input_spectrum.o                            output_ascii_hex.o output_atmel_generic.o output_dec_binary.o output_emon52.o output_fastload.o output_four_packed_code.o output_intel.o output_mos_tech.o output_srecord.o output_os65v.o output_signetics.o output_spasm.o output_tektronix.o output_tektronix_extended.o output_ti_tagged.o output_wilson.o output_cosmac.o output_fairchild.o output_formatted_binary.o output_needham.o output_spectrum.o progname.o quit.o prefix.o normal.o about.dll 
	g++.exe -o hexplo hexplorer.o container.o undo.o position.o fonts.o macro.o filetype.o mclip.o tool.o hex_common.o rsrc.o srec_win_quit.o             libdis.o i386_invariant.o i386.o MessageDigest.o sha.o md5.o ripemd.o mediaaccess.o record.o input.o output.o input_file.o output_file.o                       input_ascii_hex.o input_atmel_generic.o input_dec_binary.o input_emon52.o input_fastload.o input_four_packed_code.o input_intel.o input_mos_tech.o input_srecord.o input_os65v.o input_signetics.o input_spasm.o input_tektronix.o input_tektronix_extended.o input_ti_tagged.o input_wilson.o input_cosmac.o input_fairchild.o input_formatted_binary.o input_needham.o input_spectrum.o                     output_ascii_hex.o output_atmel_generic.o output_dec_binary.o output_emon52.o output_fastload.o output_four_packed_code.o output_intel.o output_mos_tech.o output_srecord.o output_os65v.o output_signetics.o output_spasm.o output_tektronix.o output_tektronix_extended.o output_ti_tagged.o output_wilson.o output_cosmac.o output_fairchild.o output_formatted_binary.o output_needham.o output_spectrum.o progname.o quit.o prefix.o normal.o -lcomctl32 -lole32 -luuid -lwininet -mwindows

hexplorer.o : hexplorer.cpp hexplorer.h
	g++.exe -c hexplorer.cpp $(SWITCHES)
container.o : container.h container.cpp
	g++.exe -c container.cpp $(SWITCHES)
undo.o : undo.h undo.cpp
	g++.exe -c undo.cpp $(SWITCHES)
position.o : position.h position.cpp
	g++.exe -c position.cpp $(SWITCHES)
fonts.o : fonts.h fonts.cpp
	g++.exe -c fonts.cpp $(SWITCHES)
macro.o : macro.h macro.cpp
	g++.exe -c macro.cpp $(SWITCHES)
filetype.o : filetype.h filetype.cpp
	g++.exe -c filetype.cpp $(SWITCHES)
mclip.o : mclip.h mclip.cpp
	g++.exe -c mclip.cpp $(SWITCHES)
tool.o : tool.h tool.cpp
	g++.exe -c tool.cpp $(SWITCHES)
hex_common.o : hex_common.h hex_common.cpp
	g++.exe -c hex_common.cpp $(SWITCHES)
libdis.o : libdisasm/libdis.h libdisasm/libdis.c
	gcc -c libdisasm/libdis.c $(SWITCHES)
i386_invariant.o : libdisasm/i386_invariant.c libdisasm/i386_opcode.h
	gcc -c libdisasm/i386_invariant.c $(SWITCHES)
i386.o : libdisasm/i386.h libdisasm/i386.c
	gcc -c libdisasm/i386.c $(SWITCHES)
mediaaccess.o : mediaaccess.h mediaaccess.cpp
	g++ -c mediaaccess.cpp $(SWITCHES)
srec_win_quit.o : srec_win_quit.h srec_win_quit.cpp
	g++ -c srec_win_quit.cpp $(SWITCHES)



rsrc.o : rsrc.res
	windres.exe -o rsrc.o rsrc.res
rsrc.res : rsrc.rc
	c:/bc45/bin/brcc32 -forsrc.res rsrc.rc

about.dll : about.o logo.o
	ld -o about.dll -shared about.o logo.o
about.o : about.cpp
	g++ -c about.cpp
logo.o : logo.res
	windres.exe -o logo.o logo.res
logo.res : logo.rc
	c:/bc45/bin/brcc32 logo.rc

MessageDigest.o : hash/MessageDigest.cpp hash/MessageDigest.h
	g++.exe -c hash/MessageDigest.cpp $(SWITCHES)
sha.o : hash/sha.cpp hash/sha.h
	g++.exe -c hash/sha.cpp $(SWITCHES)
md5.o : hash/md5.cpp hash/md5.h
	g++.exe -c hash/md5.cpp $(SWITCHES)
ripemd.o : hash/ripemd.cpp hash/ripemd.h
	g++.exe -c hash/ripemd.cpp $(SWITCHES)


progname.o : srecord-1.16/lib/common/progname.cc srecord-1.16/include/progname.h
	g++ -c srecord-1.16/lib/common/progname.cc $(SWITCHES)
quit.o : srecord-1.16/lib/common/quit.cc srecord-1.16/include/quit.h
	g++ -c srecord-1.16/lib/common/quit.cc $(SWITCHES)
prefix.o : srecord-1.16/lib/common/quit/prefix.cc srecord-1.16/include/quit/prefix.h
	g++ -c srecord-1.16/lib/common/quit/prefix.cc $(SWITCHES)
normal.o : srecord-1.16/lib/common/quit/normal.cc srecord-1.16/include/quit/normal.h
	g++ -c srecord-1.16/lib/common/quit/normal.cc $(SWITCHES)
record.o : srecord-1.16/lib/srec/record.cc srecord-1.16/include/srec/record.h
	g++ -c srecord-1.16/lib/srec/record.cc $(SWITCHES)
input.o : srecord-1.16/lib/srec/input.cc srecord-1.16/include/srec/input.h
	g++ -c srecord-1.16/lib/srec/input.cc $(SWITCHES)
output.o : srecord-1.16/lib/srec/output.cc srecord-1.16/include/srec/output.h
	g++ -c srecord-1.16/lib/srec/output.cc $(SWITCHES)
input_file.o : srecord-1.16/lib/srec/input/file.cc srecord-1.16/include/srec/input/file.h
	g++ -c srecord-1.16/lib/srec/input/file.cc $(SWITCHES) -o input_file.o 
output_file.o : srecord-1.16/lib/srec/output/file.cc srecord-1.16/include/srec/output/file.h
	g++ -c srecord-1.16/lib/srec/output/file.cc $(SWITCHES) -o output_file.o 

input_ascii_hex.o : srecord-1.16/lib/srec/input/file/ascii_hex.cc srecord-1.16/include/srec/input/file/ascii_hex.h
	g++ -c srecord-1.16/lib/srec/input/file/ascii_hex.cc $(SWITCHES) -o input_ascii_hex.o
input_atmel_generic.o : srecord-1.16/lib/srec/input/file/atmel_generic.cc srecord-1.16/include/srec/input/file/atmel_generic.h
	g++ -c srecord-1.16/lib/srec/input/file/atmel_generic.cc $(SWITCHES) -o input_atmel_generic.o
input_dec_binary.o : srecord-1.16/lib/srec/input/file/dec_binary.cc srecord-1.16/include/srec/input/file/dec_binary.h
	g++ -c srecord-1.16/lib/srec/input/file/dec_binary.cc $(SWITCHES) -o input_dec_binary.o
input_emon52.o : srecord-1.16/lib/srec/input/file/emon52.cc srecord-1.16/include/srec/input/file/emon52.h
	g++ -c srecord-1.16/lib/srec/input/file/emon52.cc $(SWITCHES) -o input_emon52.o
input_fastload.o : srecord-1.16/lib/srec/input/file/fastload.cc srecord-1.16/include/srec/input/file/fastload.h
	g++ -c srecord-1.16/lib/srec/input/file/fastload.cc $(SWITCHES) -o input_fastload.o
input_four_packed_code.o : srecord-1.16/lib/srec/input/file/four_packed_code.cc srecord-1.16/include/srec/input/file/four_packed_code.h
	g++ -c srecord-1.16/lib/srec/input/file/four_packed_code.cc $(SWITCHES) -o input_four_packed_code.o
input_intel.o : srecord-1.16/lib/srec/input/file/intel.cc srecord-1.16/include/srec/input/file/intel.h
	g++ -c srecord-1.16/lib/srec/input/file/intel.cc $(SWITCHES) -o input_intel.o
input_mos_tech.o : srecord-1.16/lib/srec/input/file/mos_tech.cc srecord-1.16/include/srec/input/file/mos_tech.h
	g++ -c srecord-1.16/lib/srec/input/file/mos_tech.cc $(SWITCHES) -o input_mos_tech.o
input_srecord.o : srecord-1.16/lib/srec/input/file/srecord.cc srecord-1.16/include/srec/input/file/srecord.h
	g++ -c srecord-1.16/lib/srec/input/file/srecord.cc $(SWITCHES) -o input_srecord.o
input_os65v.o : srecord-1.16/lib/srec/input/file/os65v.cc srecord-1.16/include/srec/input/file/os65v.h
	g++ -c srecord-1.16/lib/srec/input/file/os65v.cc $(SWITCHES) -o input_os65v.o
input_signetics.o : srecord-1.16/lib/srec/input/file/signetics.cc srecord-1.16/include/srec/input/file/signetics.h
	g++ -c srecord-1.16/lib/srec/input/file/signetics.cc $(SWITCHES) -o input_signetics.o
input_spasm.o : srecord-1.16/lib/srec/input/file/spasm.cc srecord-1.16/include/srec/input/file/spasm.h
	g++ -c srecord-1.16/lib/srec/input/file/spasm.cc $(SWITCHES) -o input_spasm.o
input_tektronix.o : srecord-1.16/lib/srec/input/file/tektronix.cc srecord-1.16/include/srec/input/file/tektronix.h
	g++ -c srecord-1.16/lib/srec/input/file/tektronix.cc $(SWITCHES) -o input_tektronix.o
input_tektronix_extended.o : srecord-1.16/lib/srec/input/file/tektronix_extended.cc srecord-1.16/include/srec/input/file/tektronix_extended.h
	g++ -c srecord-1.16/lib/srec/input/file/tektronix_extended.cc $(SWITCHES) -o input_tektronix_extended.o
input_ti_tagged.o : srecord-1.16/lib/srec/input/file/ti_tagged.cc srecord-1.16/include/srec/input/file/ti_tagged.h
	g++ -c srecord-1.16/lib/srec/input/file/ti_tagged.cc $(SWITCHES) -o input_ti_tagged.o
input_wilson.o : srecord-1.16/lib/srec/input/file/wilson.cc srecord-1.16/include/srec/input/file/wilson.h
	g++ -c srecord-1.16/lib/srec/input/file/wilson.cc $(SWITCHES) -o input_wilson.o
input_cosmac.o : srecord-1.16/lib/srec/input/file/cosmac.cc srecord-1.16/include/srec/input/file/cosmac.h
	g++ -c srecord-1.16/lib/srec/input/file/cosmac.cc $(SWITCHES) -o input_cosmac.o
input_fairchild.o : srecord-1.16/lib/srec/input/file/fairchild.cc srecord-1.16/include/srec/input/file/fairchild.h
	g++ -c srecord-1.16/lib/srec/input/file/fairchild.cc $(SWITCHES) -o input_fairchild.o
input_formatted_binary.o : srecord-1.16/lib/srec/input/file/formatted_binary.cc srecord-1.16/include/srec/input/file/formatted_binary.h
	g++ -c srecord-1.16/lib/srec/input/file/formatted_binary.cc $(SWITCHES) -o input_formatted_binary.o
input_needham.o : srecord-1.16/lib/srec/input/file/needham.cc srecord-1.16/include/srec/input/file/needham.h
	g++ -c srecord-1.16/lib/srec/input/file/needham.cc $(SWITCHES) -o input_needham.o
input_spectrum.o : srecord-1.16/lib/srec/input/file/spectrum.cc srecord-1.16/include/srec/input/file/spectrum.h
	g++ -c srecord-1.16/lib/srec/input/file/spectrum.cc $(SWITCHES) -o input_spectrum.o




output_ascii_hex.o : srecord-1.16/lib/srec/output/file/ascii_hex.cc srecord-1.16/include/srec/output/file/ascii_hex.h
	g++ -c srecord-1.16/lib/srec/output/file/ascii_hex.cc $(SWITCHES) -o output_ascii_hex.o
output_atmel_generic.o : srecord-1.16/lib/srec/output/file/atmel_generic.cc srecord-1.16/include/srec/output/file/atmel_generic.h
	g++ -c srecord-1.16/lib/srec/output/file/atmel_generic.cc $(SWITCHES) -o output_atmel_generic.o
output_dec_binary.o : srecord-1.16/lib/srec/output/file/dec_binary.cc srecord-1.16/include/srec/output/file/dec_binary.h
	g++ -c srecord-1.16/lib/srec/output/file/dec_binary.cc $(SWITCHES) -o output_dec_binary.o
output_emon52.o : srecord-1.16/lib/srec/output/file/emon52.cc srecord-1.16/include/srec/output/file/emon52.h
	g++ -c srecord-1.16/lib/srec/output/file/emon52.cc $(SWITCHES) -o output_emon52.o
output_fastload.o : srecord-1.16/lib/srec/output/file/fastload.cc srecord-1.16/include/srec/output/file/fastload.h
	g++ -c srecord-1.16/lib/srec/output/file/fastload.cc $(SWITCHES) -o output_fastload.o
output_four_packed_code.o : srecord-1.16/lib/srec/output/file/four_packed_code.cc srecord-1.16/include/srec/output/file/four_packed_code.h
	g++ -c srecord-1.16/lib/srec/output/file/four_packed_code.cc $(SWITCHES) -o output_four_packed_code.o
output_intel.o : srecord-1.16/lib/srec/output/file/intel.cc srecord-1.16/include/srec/output/file/intel.h
	g++ -c srecord-1.16/lib/srec/output/file/intel.cc $(SWITCHES) -o output_intel.o
output_mos_tech.o : srecord-1.16/lib/srec/output/file/mos_tech.cc srecord-1.16/include/srec/output/file/mos_tech.h
	g++ -c srecord-1.16/lib/srec/output/file/mos_tech.cc $(SWITCHES) -o output_mos_tech.o
output_srecord.o : srecord-1.16/lib/srec/output/file/srecord.cc srecord-1.16/include/srec/output/file/srecord.h
	g++ -c srecord-1.16/lib/srec/output/file/srecord.cc $(SWITCHES) -o output_srecord.o
output_os65v.o : srecord-1.16/lib/srec/output/file/os65v.cc srecord-1.16/include/srec/output/file/os65v.h
	g++ -c srecord-1.16/lib/srec/output/file/os65v.cc $(SWITCHES) -o output_os65v.o
output_signetics.o : srecord-1.16/lib/srec/output/file/signetics.cc srecord-1.16/include/srec/output/file/signetics.h
	g++ -c srecord-1.16/lib/srec/output/file/signetics.cc $(SWITCHES) -o output_signetics.o
output_spasm.o : srecord-1.16/lib/srec/output/file/spasm.cc srecord-1.16/include/srec/output/file/spasm.h
	g++ -c srecord-1.16/lib/srec/output/file/spasm.cc $(SWITCHES) -o output_spasm.o
output_tektronix.o : srecord-1.16/lib/srec/output/file/tektronix.cc srecord-1.16/include/srec/output/file/tektronix.h
	g++ -c srecord-1.16/lib/srec/output/file/tektronix.cc $(SWITCHES) -o output_tektronix.o
output_tektronix_extended.o : srecord-1.16/lib/srec/output/file/tektronix_extended.cc srecord-1.16/include/srec/output/file/tektronix_extended.h
	g++ -c srecord-1.16/lib/srec/output/file/tektronix_extended.cc $(SWITCHES) -o output_tektronix_extended.o
output_ti_tagged.o : srecord-1.16/lib/srec/output/file/ti_tagged.cc srecord-1.16/include/srec/output/file/ti_tagged.h
	g++ -c srecord-1.16/lib/srec/output/file/ti_tagged.cc $(SWITCHES) -o output_ti_tagged.o
output_wilson.o : srecord-1.16/lib/srec/output/file/wilson.cc srecord-1.16/include/srec/output/file/wilson.h
	g++ -c srecord-1.16/lib/srec/output/file/wilson.cc $(SWITCHES) -o output_wilson.o
output_cosmac.o : srecord-1.16/lib/srec/output/file/cosmac.cc srecord-1.16/include/srec/output/file/cosmac.h
	g++ -c srecord-1.16/lib/srec/output/file/cosmac.cc $(SWITCHES) -o output_cosmac.o
output_fairchild.o : srecord-1.16/lib/srec/output/file/fairchild.cc srecord-1.16/include/srec/output/file/fairchild.h
	g++ -c srecord-1.16/lib/srec/output/file/fairchild.cc $(SWITCHES) -o output_fairchild.o
output_formatted_binary.o : srecord-1.16/lib/srec/output/file/formatted_binary.cc srecord-1.16/include/srec/output/file/formatted_binary.h
	g++ -c srecord-1.16/lib/srec/output/file/formatted_binary.cc $(SWITCHES) -o output_formatted_binary.o
output_needham.o : srecord-1.16/lib/srec/output/file/needham.cc srecord-1.16/include/srec/output/file/needham.h
	g++ -c srecord-1.16/lib/srec/output/file/needham.cc $(SWITCHES) -o output_needham.o
output_spectrum.o : srecord-1.16/lib/srec/output/file/spectrum.cc srecord-1.16/include/srec/output/file/spectrum.h
	g++ -c srecord-1.16/lib/srec/output/file/spectrum.cc $(SWITCHES) -o output_spectrum.o


