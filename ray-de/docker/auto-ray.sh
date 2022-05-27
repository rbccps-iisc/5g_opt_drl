RAY_COMMAND=${1:-up}
NUM_WORKER_NODES=${2:-3}
RAY_MIN_WORKERS=${3:--1}

image_name="ray-de"

if [[ $RAY_COMMAND == "up" ]]
then
    if [[ ! $(docker image ls | grep $image_name) ]]
    then
        bash docker/build.sh $image_name
    fi

    cd ./docker
    docker-compose up -d --scale ray-de-worker=$NUM_WORKER_NODES
    cd ..

    if [[ $RAY_MIN_WORKERS == "-1" ]]
    then
        python3 docker/update-ray-autoscaler-config.py
    else
        python3 docker/update-ray-autoscaler-config.py \
            --min_workers $RAY_MIN_WORKERS
    fi

    ray up configs/ray-autoscaler.yaml -y
else
    ray down configs/ray-autoscaler.yaml -y
    cd docker
    docker-compose down
    cd ..
fi