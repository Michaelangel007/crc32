# "CRC32 Demystified"
# https://github.com/Michaelangel007/crc32
#
# Michaelangel007
# Copyleft (C) 2017


# http://stackoverflow.com/questions/5377297/how-to-manually-call-another-target-from-a-make-target
# Determine this makefile's path.
# Be sure to place this BEFORE `include` directives, if any.
THIS_FILE := $(lastword $(MAKEFILE_LIST))

all: bin jar usage

# Targets & Executables
usage:
	@echo ""
	@echo "Make Targets..."
	@echo ""
	@echo "  make clean  # Clean bin/ and java/bin"
	@echo "  make bin    # Build bin/ files"
	@echo "  make jar    # Build java/bin/.jar file"
	@echo "  make java   # Run .jar file"
	@echo ""
	@echo "Executables..."
	@echo "  bin/crc32"
	@echo "  bin/enum"
	@echo "  bin/tables"
	@echo "  bin/trace"
	@echo "  bin/bret_crc32.jar"

.PHONY: clean
.PHONY: clean_bin
.PHONY: clean_java
.PHONY: java
.PHONY: java_dirs

# @ silient command
# -f ignore missing miles
#
# Note:
#    .SILENT: clean
# is obsolete. Use `@` instead.
#
# Reference:
# * http://stackoverflow.com/questions/3148492/suppress-messages-in-make-clean-makefile-silent-remove
clean_bin:
	@echo "Cleaning bin/..."
	@rm -f bin/*

clean_java:
	@echo "Cleaning java/bin/..."
	@rm -f java/bin/*.class
	@rm -f java/bin/crc32/*.class
	@rm -f java/bin/*.jar

clean:
	@$(MAKE) -f $(THIS_FILE) clean_bin
	@$(MAKE) -f $(THIS_FILE) clean_java


# === C++ ===

DEP_H=$(wildcard src/*.h)
CC=g++
C_INC=-Isrc/
C_FLAGS=$(C_INC)

bin: bin_dir bin/crc32 bin/enum bin/tables bin/trace bin/crc33

bin_dir:
	@mkdir -p bin

bin/crc32: src/crc32.cpp $(DEP_H)
	$(CC) $(C_FLAGS) $< -o $@

bin/enum: src/enum_crc32.cpp $(DEP_H)
	$(CC) $(C_FLAGS) $< -o $@

bin/tables: src/tables_crc32.cpp $(DEP_H)
	$(CC) $(C_FLAGS) $< -o $@

bin/trace: src/trace_crc32.cpp $(DEP_H)
	$(CC) $(C_FLAGS) $< -o $@

bin/crc33: src/crc33.cpp $(DEP_H)
	$(CC) $(C_FLAGS) $< -o $@

# === Java ===

# -verbose   Extra logging
# -cp        Class Path for Import Path
JAVA_INC=-cp bin
JAVA_FLAGS=-d bin $(JAVA_INC)

DEP_JAVA= java/src/crc32/broken.java  java/src/crc32/normal.java  java/src/crc32/reflect.java
DEP_CLASS=java/bin/crc32/broken.class java/bin/crc32/normal.class java/bin/crc32/reflect.class

SRC_JAVA= java/src/bret_crc32.java
SRC_CLASS=java/bin/bret_crc32.class
JAR_FILE=      bin/bret_crc32.jar
jar: java_dirs $(DEP_CLASS) $(SRC_CLASS) $(JAR_FILE)

java_dirs:
	@mkdir -p java/bin

java/bin/crc32/broken.class: java/src/crc32/broken.java
	cd java && javac $(JAVA_FLAGS) ../$< && cd ..

java/bin/crc32/normal.class: java/src/crc32/normal.java
	cd java && javac $(JAVA_FLAGS) ../$< && cd ..

java/bin/crc32/reflect.class: java/src/crc32/reflect.java
	cd java && javac $(JAVA_FLAGS) ../$< && cd ..

java/bin/bret_crc32.class:    java/src/bret_crc32.java
	cd java && javac $(JAVA_FLAGS) ../$< && cd ..

# jar {ctxui}[vfmn0Me] [jar-file] [manifest-file] [entry-point] [-C dir] files ...
# files  list of *.class filenames
# c      Create .jar
# f      .JAR file name
# v      Verbose output status
# e      Optional: Application entry point
#        If not specified default manifest doesn't specify it
#
#        Manifest
#
#           echo "<MainClass>: <Path.To.Class>" > MANIFEST.txt
#           jar <file.jar> MANIFEST.txt files ...
#
#        Or
#
#           jar -e  <file.jar> <EntryPointClass> files ...
#
# References:
# * http://stackoverflow.com/questions/36281181/making-manifest-file-in-java-by-command-prompt
# * http://www.skylit.com/javamethods/faqs/createjar.html


# Note: .JAR file contents relative to current directory
#
# There are two ways to run the java executable
#
#     # With OUT .class extension
#     cd java/bin
#     java bret_crc32
#
# We package all the class files together into a single .jar file
#
#     # WITH .jar extension
#     java -jar bin/bret_crc32.jar
#
bin/bret_crc32.jar: $(SRC_JAVA) $(DEP_JAVA)
	cd java/bin && jar cvfe ../../$(JAR_FILE) bret_crc32 *.class crc32/*.class && cd ../..

# Run .JAR file
java:
	java -jar $(JAR_FILE)

