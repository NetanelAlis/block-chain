# MTA Coin on Containers

## Objective
This project focuses on practicing Inter-Process Communication (IPC) in a container-based development environment. The system includes a server and multiple miner containers, which communicate through named pipes, to simulate a blockchain mining process.

## Overview
The application includes two Docker containers:
 - *Server Container*  
   The server Process is the first process to run and is in charge of managing the blockchain.
   the server will register new miners who wants to mine new blocks,
   will verify all new blocks suggested by any active miner, thus preventing any out of order block to be linked to the blockchain,
   when a new valid block was mined by a miner and aprooved by the server the server will link it to the blockchain making it its new head
   and will inform all active miners of the new blockchain head.

 - *Miner Containers*  
   The miner processes will start running one after the other after the server is already running.
   Each new miner will send a subscription request to the server via a dedicated server pipe where the miner will send the server his ID,
   should the subscription rquest be accepted by the server the server will open a new pipe from him to the new miner where the current blockchain head will be sent to the miner.
   The miner process will be using the data recieved form the server's blockchain head and crc32 hash function to create a new block, and once the miner finds a new block meeting the             requirements of the server, the miner will send the new block to the server using the server pipe.

### Key Features:
- The server manages subscriptions and block validation.
- Miners use named pipes to communicate with the server.
- The system supports dynamic scaling of miners based on input parameters.

## Getting Started

### Prerequisites
Ensure the following tools are installed:
- Docker
- Bash shell
- Sudo privileges

### Running the Application
To launch the system, follow these steps:

1. Make the launcher script executable:
   ```bash
   chmod +x launcher.sh

2. **Run the launcher.sh script with two parameters**:

    difficulty level (e.g., 5)
    number of miners (e.g., 3)
   
Example:

If you want to run the system with a difficulty level of 5 and 3 miners, the command would be:

```bash
   ./launcher.sh 5 3
```
   
Script Behavior

   - The script will pull the Docker images from Docker Hub. Ensure you're logged into Docker Hub before running the script.
   - The script requires sudo permissions to run Docker commands.
   - A mounted directory (mnt/mta/) will be created in the current working directory to store pipes and the configuration file.
   - The configuration file (mtacoin.conf) is generated automatically and contains the difficulty level and the miner IDs.
   - The script removes old named pipes and updates the configuration file with new parameters each time it runs.
