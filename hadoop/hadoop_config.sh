#!/bin/bash

# 4. Set env vars
cd 
echo "PATH=/home/hadoop/hadoop/bin:/home/hadoop/hadoop/sbin:\$PATH" >> .profile
echo "export HADOOP_HOME=\$HOME/hadoop" >> .bashrc
echo "export PATH=\$PATH:\$HADOOP_HOME/bin:\$HADOOP_HOME/sbin" >> .bashrc

# 5. Set java home
JAVA_HOME=# Add here your java dir e.g.: /usr/lib/jvm/java-8-openjdk/jre
echo "export JAVA_HOME=$JAVA_HOME" >> .bashrc

# 6. Copy config files
HADOOP_CONFIG_DIR=$HOME/hadoop/etc/hadoop/
cp hadoop_conf/* $HADOOP_CONFIG_DIR


