# ==========================================================
# this file contains the data sturcture for the compiler
# Note: this is tools specific and needs to be adjusted
# ==========================================================

import json
from collections import OrderedDict

def getCompilerToolName(args):
	return "arm-none-eabi-gcc"

def get_platform(args, my_url):
	tool = OrderedDict([
		('name', "Energia CC13xx boards"),
		('architecture', args.arch),
		('version', args.version),
		('category', "Energia"),
		('url', my_url + args.arch + "-" + args.version + ".tar.bz2"),
		('archiveFileName', args.arch + "-" + args.version + ".tar.bz2"),
		('checksum', "0"),
		('size', ""),
		("boards", [
			{"name": "LAUNCHXL_CC1310"},
			{"name": "LAUNCHXL_CC1350"},
			{"name": "CC1350STK"}
		]),
		("toolsDependencies", []),
	])
	return tool


def init_tools_data(args, my_url):
	ctn = getCompilerToolName(args)
	tool = OrderedDict([
		('name', ctn),
		('version', args.cversion),
		('systems', [
			{
				'host': 'i686-mingw32',
				'url': my_url + "windows/" + args.cname + "-" + args.cversion + '-windows.tar.bz2',
				'archiveFileName': args.cname + "-" + args.cversion + '-windows.tar.bz2',
			},
			{
				'host': 'x86_64-apple-darwin',
				'url': my_url + 'macosx/' + args.cname + "-" + args.cversion + '-mac.tar.bz2',
				'archiveFileName': args.cname + "-" + args.cversion + '-mac.tar.bz2',
			},
			{
				'host': 'x86_64-pc-linux-gnu',
				'url': my_url + 'linux64/' + args.cname + "-" + args.cversion + '-x86_64-pc-linux-gnu.tar.bz2',
				'archiveFileName': args.cname + "-" + args.cversion + '-x86_64-pc-linux-gnu.tar.bz2',
			},
		])
	])
	return tool
