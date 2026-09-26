"""
DataTable 원본 JSON을 CSV로 바꾼다. (언리얼 DataTable 임포트, 엑셀 편집용)

사용법:  python Data/json_to_csv.py Data/DT_UIText.json
결과:    같은 이름의 .csv (UTF-8 BOM, 엑셀에서 한글이 깨지지 않는다)

- 첫 열은 행 이름(Name)이다.
- 문자열 안의 줄바꿈은 \\n 두 글자로 저장한다. 게임이 읽을 때 실제 줄바꿈으로 바꾼다. (VXText::Get)
- 배열·구조체 값(예: DT_Waves의 Spawns)은 CSV로 표현하기 어려워 JSON 그대로 쓰는 것을 권한다.
"""
import csv
import json
import os
import sys


def to_cell(value):
    if isinstance(value, str):
        return value.replace("\r\n", "\n").replace("\n", "\\n")
    if isinstance(value, (list, dict)):
        return json.dumps(value, ensure_ascii=False)
    return value


def main(path):
    rows = json.load(open(path, encoding="utf-8"))
    columns = ["Name"] + [key for key in rows[0].keys() if key != "Name"]
    out = os.path.splitext(path)[0] + ".csv"
    with open(out, "w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f, quoting=csv.QUOTE_ALL)
        writer.writerow(columns)
        for row in rows:
            writer.writerow([to_cell(row.get(column, "")) for column in columns])
    print(f"{out}: {len(rows)} rows")


if __name__ == "__main__":
    for arg in sys.argv[1:]:
        main(arg)
