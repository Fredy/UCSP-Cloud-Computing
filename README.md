# UCSP-Cloud-Computing

# 1. Configure hosts file
# 2. Add master key to authorized_keys

# 3. Install hadoop

Run this:
```
chmod +x install_hadoop.sh
./install_hadoop.sh
```

# 4.  Hadoop config
You must need to add your java dir in `JAVA_HOME` at `hadoop_conf/hadoop_config.sh`
Also add the nodes in the file `hadoop_conf/workers`

Run this:
```
chmod +x hadoop_config.sh
./hadoop_config.sh
```

