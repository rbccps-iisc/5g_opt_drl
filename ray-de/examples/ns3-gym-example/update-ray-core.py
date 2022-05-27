import ray
import shutil
from pathlib import Path

ray_dir = Path(ray.__file__).parent

dst_file = ray_dir/"rllib/policy/sample_batch.py"
src_file = Path("sample_batch.py.replace")

with open(dst_file, "r") as f:
    dst_source_code = f.read()
with open(src_file, "r") as f:
    src_source_code = f.read()

if src_source_code != dst_source_code:
    print(f"Difference in {dst_file.name} observed. Updating it.")
    shutil.copy(src=src_file, dst=dst_file)
    print("Done")