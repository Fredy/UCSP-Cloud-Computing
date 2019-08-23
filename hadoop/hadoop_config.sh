#!/bin/bash

BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# 4. Set env vars
cd 
echo "PATH=/home/hadoop/hadoop/bin:/home/hadoop/hadoop/sbin:\$PATH" >> .profile
echo "export HADOOP_HOME=\$HOME/hadoop" >> .bashrc
echo "export PATH=\$PATH:\$HADOOP_HOME/bin:\$HADOOP_HOME/sbin" >> .bashrc

# 5. Set java home
# -

# 6. Copy config files
HADOOP_CONFIG_DIR=$HOME/hadoop/etc/hadoop/
cp "$BASEDIR/hadoop_conf/"* $HADOOP_CONFIG_DIR


