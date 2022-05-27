import re
import subprocess
import argparse


def main(
    get_ips_script="docker/get-container-ips.sh",
    docker_network_name="docker_ray-de",
    input_autoscaler_yaml="configs/ray-autoscaler.yaml",
    output_autoscaler_yaml=None,
    min_workers=None,
):

    ips = subprocess.run(
        ["bash", get_ips_script, docker_network_name],
        capture_output=True,
        text=True
    ).stdout.strip().split("\n")

    head_ip = [
        i.split(":")[1].strip().split("/")[0] for i in ips if "head" in i
    ][0]

    # list converted to string for string substitution later on
    worker_ips = [
        i.split(":")[1].strip().split("/")[0] for i in ips if "worker" in i
    ]
    max_workers = len(worker_ips)
    worker_ips = str(worker_ips).replace("'","")

    with open(input_autoscaler_yaml, "r") as f:
        config_text = f.read()

    # look behind for head_ip:<space> and then check for ip format
    pat = re.compile("(?<=head_ip: )\d+.\d+.\d+.\d+")
    config_text = re.sub(pat, head_ip, config_text)

    # look behind for worker_ips:<space> and check for any length entry in []
    pat = re.compile("(?<=worker_ips: )\[.*\]")
    config_text = re.sub(pat, worker_ips, config_text)

    # look behind for max_workers:<space> and check for any length digit
    pat = re.compile("(?<=max_workers: )\d+")
    config_text = re.sub(pat, str(max_workers), config_text)

    if min_workers is None:
        min_workers = max_workers
    pat = re.compile("(?<=min_workers: )\d+")
    config_text = re.sub(pat, str(min_workers), config_text)

    if output_autoscaler_yaml is None:
        output_autoscaler_yaml = input_autoscaler_yaml

    with open(output_autoscaler_yaml, "w") as f:
        f.write(config_text)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Automatically update ray autoscaler config file"\
        " with current docker container ips"
    )

    parser.add_argument(
        "--get_ips_script", 
        default="docker/get-container-ips.sh",
        help="Path of shell script to get container ips",
    )

    parser.add_argument(
        "--docker_network_name",
        default="docker_ray-de",
        help="Name of the docker network from which ips to be extracted",
    )

    parser.add_argument(
        "--input_autoscaler_yaml",
        default="configs/ray-autoscaler.yaml",
        help="input yaml file with ray autoscaler config",
    )

    parser.add_argument(
        "--output_autoscaler_yaml",
        help="output yaml file with ray autoscaler config,"\
            " overwrites the input file if not provided.",
    )

    parser.add_argument(
        "--min_workers",
        help="minimum workers to be set in autoscaler config,"\
            " set to max_workers if not provided"
    )

    args = parser.parse_args()

    main(
        args.get_ips_script,
        args.docker_network_name,
        args.input_autoscaler_yaml,
        args.output_autoscaler_yaml,
        args.min_workers,
    )