import sys, tempfile, unittest
from pathlib import Path
from subprocess import CompletedProcess, TimeoutExpired
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from availability_monitor import probe_application, write_report

class AvailabilityMonitorTests(unittest.TestCase):
    def test_successful_start_menu_and_exit(self):
        result = CompletedProcess(["railway"], 0,
            "===== RAILWAY RESERVATION SYSTEM =====\nEnter choice: Exiting system. Goodbye!\n", "")
        with patch("availability_monitor.subprocess.run", return_value=result):
            self.assertTrue(probe_application(Path("railway"), 5)["available"])

    def test_nonzero_exit_is_unavailable(self):
        result = CompletedProcess(["railway"], 1, "Enter choice: Exiting system. Goodbye!\n", "error")
        with patch("availability_monitor.subprocess.run", return_value=result):
            self.assertFalse(probe_application(Path("railway"), 5)["available"])

    def test_missing_menu_is_unavailable(self):
        result = CompletedProcess(["railway"], 0, "unexpected output", "")
        with patch("availability_monitor.subprocess.run", return_value=result):
            self.assertFalse(probe_application(Path("railway"), 5)["available"])

    def test_timeout_is_unavailable(self):
        with patch("availability_monitor.subprocess.run", side_effect=TimeoutExpired(["railway"], 5)):
            result = probe_application(Path("railway"), 5)
        self.assertFalse(result["available"])
        self.assertIn("timeout_after_5s", result["reason"])

    def test_csv_and_sample_rate(self):
        rows = [
            {"started_at_utc":"a","finished_at_utc":"b","available":True,"duration_ms":12,"exit_code":0,"reason":"ok"},
            {"started_at_utc":"c","finished_at_utc":"d","available":False,"duration_ms":10,"exit_code":1,"reason":"failed"}]
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "evidence.csv"
            self.assertEqual(write_report(report, rows), 50.0)
            self.assertIn("available", report.read_text(encoding="utf-8"))

if __name__ == "__main__": unittest.main()
