container_name=${1:-ray-de-head}
docker run -it \
	--rm \
	--shm-size 5gb \
	--name $container_name \
	ray-de:latest
