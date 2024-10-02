#!/bin/bash

# Input parameters
difficulty=${1}
numOfMiners=${2}

# Local variables
dir_name="mnt/mta/"
config_file="${dir_name}/mtacoin.conf"
current_dir=$(pwd)
server_image_and_tag="nadir0702/mtacoin_ex3:mtacoin-server"
miner_image_and_tag="nadir0702/mtacoin_ex3:mtacoin-miner"

# Number of Miner validation (positive integer)
if ! [[ "$numOfMiners" =~ ^[1-9][0-9]*$ ]]; then
    echo "Number of Miners was not a Positive Integer."
    exit 1
fi

# Pull Docker images
echo "Pulling Docker images..."
docker pull $server_image_and_tag
docker pull $miner_image_and_tag

# Create the mounted directory if it doesn't exists
if [[ ! -d $dir_name ]]; then
    mkdir -p $dir_name
    touch "$config_file"
    echo "DIFFICULTY=" > "$config_file"
    echo "NEXT_MINER_ID=0" >> "$config_file"
    echo "Mounted Directory created."
else
    # Delete server pipe
    rm -f "${dir_name}server_pipe"
fi

# Set difficulty to the parameter received from the user
sed -i "s/^DIFFICULTY=.*/DIFFICULTY=${difficulty}/" "$config_file"

# Get number of miner pipes from previous run
miner_id=$(grep '^NEXT_MINER_ID=' "$config_file" | cut -d '=' -f 2)

# Delete miner pipes from previous run
for (( i = 1; i < miner_id; i++ ))
do
    # Delete the file
    rm -f "${dir_name}miner_pipe_$i"
done

# Set Next Miner ID to 1 for the next run
sed -i "s/^NEXT_MINER_ID=.*/NEXT_MINER_ID=1/" "$config_file"

# Run server container
sudo docker run -d -v $current_dir/mnt/mta:/root/mnt/mta $server_image_and_tag

# Make sure server is up before any miner
sleep 1

# Run miner containers
for (( i = 0; i < numOfMiners; i++ ))
do
    sudo docker run -d -v $current_dir/mnt/mta:/root/mnt/mta $miner_image_and_tag
done