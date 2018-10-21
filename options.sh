#!/bin/sh

######################################################
#set project name
cfg_set_projet "numaprof"

######################################################
#test
cfg_add_enable_option --name='test' \
						--on='-DENABLE_TESTS=${ON}' \
						--doc='Disable unit tests' \
						--invert-help
#gcc test coverage
cfg_add_enable_option --name='gcc-coverage'  \
						--on='-DENABLE_GCC_COVERAGE=${ON}' \
						--doc='Enable GCC option to generate test coverage of the lib.' \
#junit xml out
cfg_add_enable_option --name='junit-output'  \
						--on='-DENABLE_JUNIT_OUTPUT=${ON}' \
						--doc='Save test output in JUnit format (only in self-test mode).' 
#valgrind
cfg_add_enable_option --name='valgrind' \
						--on='-DENABLE_VALGRIND=${ON}' \
						--doc 'Enable running unit tests into valgrind to generate reports.'
#jenkins full features
cfg_add_enable_option --name='jenkins' \
						--on-enable-inherit='--enable-valgrind --enable-junit-output --enable-debug "CXXFLAGS=-Wall -fprofile-arcs -ftest-coverage"' \
						--doc='Enable all checking modes (unit, valgrind, coverage...).' \
						--only='enable'
#webview
cfg_add_enable_option --name='webview' \
						--on='-DENABLE_WEBVIEW=${ON}' \
						--doc='Disable installation of webview (-DENABLE_WEBVIEW=no).'