#!/bin/sh

##################### FUNCTIONS ##########################

prompt()
{
	if test $NONINTERACTIVE; then
		return 0
	fi

	# BASH echo -ne "$*" >&2
	echo -n "$*" >&2
	read CMD rest
	return 0
}

# ----------------------------------------------------------------------------
# Get Yes/No
# ----------------------------------------------------------------------------
getyn()
{
	if test $NONINTERACTIVE; then
		return 0
	fi

	while prompt "$* (y/n) "
	do	case $CMD in
			[yY])	return 0
				;;
			[nN])	return 1
				;;
			*)	echo -e "\nPlease answer y or n" >&2
				;;
		esac
	done
}           


usage()
{
	echo
 	echo "Usage info"
	echo
	echo "./mkrelease.sh rev 1 0 0 tag [y|n]"
	echo
	exit 1
}


##################### MAIN ##########################


HOME=$(pwd)

. helper_functions.sh

ver=`git log -n1 --oneline | cut -d' ' -f 1`

rev=""
major=""
minor=""
patch=""
tag=""

while [ 1 ]
do
    if [ "$1" = "" ]; then
		break;
	fi

	if [ "$1" = "rev" ]; then
      	major=$2
		shift
      	minor=$2
		shift
      	rev=$2
		shift
      	patch=$2
		shift
	fi
	if [ "$1" = "tag" ]; then
		if [ "$2" = "" ]; then
			usage
		fi
      	tag=$2
		shift
	fi
	shift

done


echo
echo
echo "Build Info"
echo "   Major=$major"
echo "   Minor=$minor"
echo "   Rev  =$rev"
echo "   Tag  =$tag"

if [ "$major" = "" ] || [ "$minor" = "" ] || [ "$rev" = "" ] || [ "$tag" = "" ]; then
	echo "Error: Invalid major, minor, rev, tag options"
	echo
	usage
	exit 1
fi

rel_name="libwat-"$major"."$minor"."$rev

clear

echo
echo
echo "Build Info"
echo "   Major=$major"
echo "   Minor=$minor"
echo "   Rev  =$rev"
echo "   Tag  =$tag"
echo

echo
getyn "Build Release For: $rel_name - Tag $tag - GIT $ver "
echo
if [ $? -ne 0 ]; then
  	exit 1
fi

echo
echo
echo "Building Version = $rel_name  GIT Rev=$ver"
echo

if [ -d $rel_name ];then
	getyn "$rel_name already exists, it will be removed"
	if [ $? -ne 0 ]; then
		exit 1
	else
		rm -rf $rel_name
	fi
fi

eval "mkdir $rel_name"
if [ $? -ne 0 ]; then
	echo "Failed to create new directory"
fi

eval "cat ../../CMakeLists.txt | \
		sed s/\"SET(wat_VERSION_LT_CURRENT\".*/\"SET(wat_VERSION_LT_CURRENT $major)\"/ | \
		sed s/\"SET(wat_VERSION_LT_REVISION\".*/\"SET(wat_VERSION_LT_REVISION $minor)\"/ | \
		sed s/\"SET(wat_VERSION_LT_AGE\".*/\"SET(wat_VERSION_LT_AGE $rev)\"/ > $rel_name/CMakeLists.txt"

eval "mkdir $rel_name/src/"
eval "cp -rf ../../src/*.c $rel_name/src"
eval "mkdir $rel_name/src/base64"
eval "cp -rf ../../src/base64/*.c $rel_name/src/base64"
eval "cp -rf ../../src/base64/*.h $rel_name/src/base64"
eval "cp -rf ../../src/wat_config.h.in $rel_name/src"
eval "cp -rf ../../src/CMakeLists.txt $rel_name/src"

eval "mkdir $rel_name/src/include"
eval "cp -rf ../../src/include/*.h $rel_name/src/include"
eval "touch $rel_name/src/include/*"

eval "mkdir $rel_name/src/include/private"
eval "cp -rf ../../src/include/private/*.h $rel_name/src/include/private"
eval "touch $rel_name/src/include/private/*"

eval "mkdir $rel_name/test"
eval "cp -rf ../../test/* $rel_name/test"
eval "cp -rf ../../test/CMakeLists.txt $rel_name/test"

eval "mkdir $rel_name/build"
eval "cp -rf ../../README $rel_name/"
eval "cp -rf ../../AUTHORS $rel_name/"

eval "mkdir $rel_name/asterisk/"
eval "cp -rf ../../asterisk/* $rel_name/asterisk/"

#update changelog
# I think changelog updating has to be manual process before making a release, sometimes manual additions/adjustments are required
#cd ..
#eval "./gen_changelog.pl --project_name=LibWAT --version=${major}.${minor}.${rev} --project_dir="..""
#if [ $? -ne 0 ]; then
#	echo "Failed to generate changelog"
#	exit 1
#fi

#cd $HOME
#eval "cp -rf ../Changelog $rel_name"
#if [ $? -ne 0 ]; then
#	echo "Failed to copy changelog"
#	exit 1
#fi

cd $HOME
cd ..
gitinfo_libwat=`git log -n1 --oneline | cut -d' ' -f 1`


cd $HOME

cd $rel_name 
find . | xargs touch
echo "libwat: git ver $gitinfo_libwat" > .git_info

cd $HOME

eval "tar cfz $rel_name".tgz"  $rel_name"
if [ $? -ne 0 ]; then
	echo "Failed to create tarball"
	exit 1
fi
echo "Created $rel_name.tgz successfully"

if [ -e libwat-$major.$minor-current.tgz ]; then
	eval "rm -rf libwat-$major.$minor-current.tgz"
	if [ $? -ne 0 ]; then 
		echo "Failed to remove current tarball"
		exit 1
	fi
fi

tar cfz libwat-$major.$minor-current.tgz $rel_name
if [ $? -ne 0 ]; then
	echo "Failed to create tarball"
	exit 1
fi
echo "Created libwat-$major.$minor-current.tgz successfully"

tagname="v$major.$minor.$rev.$patch"

if [ $tag = "y" ]; then
	echo
  	cmd="git tag -m 'Tag $tagname' -a '$tagname'" 
	cd $HOME
	echo "$cmd"
	eval "$cmd"

	if [ $? -ne 0 ]; then
		echo "Failed to create new git tag"
		exit 1
	fi

	cmd="git push origin $tagname"
	echo "$cmd"
	eval "$cmd"	
	if [ $? -ne 0 ]; then
		echo "Failed to push new git tag"
		exit 1
	fi
fi

eval "scp $rel_name.tgz sangoma@ftp.sangoma.com:linux/libwat"
if [ $? -ne 0 ]; then
	echo "Failed to upload to ftp"
	exit 1
fi

eval "scp libwat-$major.$minor-current.tgz sangoma@ftp.sangoma.com:linux/libwat"
if [ $? -ne 0 ]; then
	echo "Failed to upload to ftp"
	exit 1
fi

echo
echo "Release Done"
