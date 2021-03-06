# Make VCProj
#
# March 2006
# Markus Chimani, markus.chimani@cs.uni-dortmund.de
#########################################################

import os, sys, fnmatch, ConfigParser

class stuff:
	def __init__(self, ttag, tpath, tpats):
		self.tag, self.path, self.pats = ttag, tpath, tpats

def bailout(msg):
	print msg
	print 'Please use the original makeVCProj.config as a template'
	sys.exit()
	
def loadConfig(sect, key, noError = False ):
	if config.has_option(sect, key):
		v = config.get(sect, key)
		print '   [', sect, ']', key, '=', v
		return v
	else:
		if noError:
			return None		
		else:
			bailout('Option "' + key + '" in section "' + sect + '" is missing')

#########################################################
# LOAD CONFIGURATION

config = ConfigParser.ConfigParser()
print 'Loading makeVCProj.config...'

try:
	config.readfp( open('makeVCProj.config') )
except IOError:
	bailout('makeVCProj.config not found')

if not config.has_section('GENERAL'):
	bailout('Section "GENERAL" is missing')
if not config.has_section('COIN'):
	bailout('Section "COIN" is missing')

#########################################################
# CONFIGS

# Filenames
filename_vcproj =  loadConfig('GENERAL', 'projectFile')
filename_template = loadConfig('GENERAL', 'templateFile')
addIncludes = ''
addDefines = ''

useOwnLpSolver = loadConfig('GENERAL', 'useOwnLpSolver', 'false').startswith('t')
if useOwnLpSolver:
	addDefines += 'OGDF_OWN_LPSOLVER;'
	
useCoin = loadConfig('COIN', 'useCoin').startswith('t')
if useCoin:
	coinIncl = loadConfig('COIN', 'coinIncl')
	solver_name = loadConfig('COIN', 'solver_name')
	solver_incl = loadConfig('COIN', 'solver_incl')

	addDefines += 'USE_COIN;'+solver_name+';'
	addIncludes += coinIncl+';'
	if solver_incl.strip() != '':
		addIncludes += solver_incl+';'

addDefines = addDefines[:-1]
addIncludes = addIncludes[:-1]
defineTag = '<<DEFINETAG>>'
includeTag = '<<INCLUDETAG>>'

# Params are: 
# - Tag in template-File
# - Directory to start search & subfilters from
# - File Patterns
cppStuff = stuff( '<<CPPTAG>>', '.\\src', [ '*.c', '*.cpp' ] )
hStuff = stuff( '<<HTAG>>', '.\\ogdf', [ '*.h' ] )
hLegacyStuff = stuff( '<<HLEGACYTAG>>', '.\\ogdf_legacy', [ '*.h' ] )
otherStuff = stuff( '<<OTHERTAG>>', '.', [ ] )

includeStuff = stuff

stuff = [ cppStuff, hStuff, hLegacyStuff, otherStuff ]


#########################################################
#########################################################
## only code below...

# just the def. nothing happens yet
def Walk( curdir, pats, intro ):
	names = os.listdir( curdir)
	names.sort()
	
	for name in names:
		if name.startswith('.') or name.startswith('_') or (name=='legacy' and not includeLegacyCode):
			continue

		outpath = curdir + '\\' + name
		fullname = os.path.normpath(outpath)
		
		if os.path.isdir(fullname) and not os.path.islink(fullname):
			if Walk( outpath, pats, intro + '<Filter Name="' + name + '">\n'):
				intro = ''
				vcproj.write('</Filter>\n')
		else:
			for pat in pats:
				if fnmatch.fnmatch(name, pat):
					if len(intro)>0:
						vcproj.write(intro)
						intro = ''
					vcproj.write('<File RelativePath="' + outpath + '"> </File>\n')
	return len(intro) == 0

##########################################
## Main below...

print 'Generating VCProj...'

includeLegacyCode = 0;
if len(sys.argv)>1 and sys.argv[1]=='legacy':
	includeLegacyCode = 1
	print '(including legacy code)'

vcproj = open(filename_vcproj,'w')
template = open(filename_template)

check = 0
for line in template:
	if check < len(stuff) and line.find(stuff[check].tag) > -1:
		if (stuff[check].tag!='<<HLEGACYTAG>>' or includeLegacyCode):
			Walk(stuff[check].path, stuff[check].pats, '')
		check = check + 1
	elif line.find(defineTag) > -1:
		vcproj.write(line.replace(defineTag,addDefines,1))
	elif line.find(includeTag) > -1:
		vcproj.write(line.replace(includeTag,addIncludes,1))
	else:
		vcproj.write(line)

template.close()
vcproj.close()
		 
print 'VCProj generated'
