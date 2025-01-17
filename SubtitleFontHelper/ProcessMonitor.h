
#pragma once

#include <string>
#include <functional>
#include <cstdint>
#include <exception>

typedef std::function<void(const std::wstring& path, uint32_t process_id)> ProcessCreatedCallback;

class _impl_ProcessMonitor;

class ProcessMonitor {
private:
	_impl_ProcessMonitor* impl;
public:
	ProcessMonitor();
	~ProcessMonitor();

	void AddProcessName(const std::wstring& process);
	void RemoveProcessName(const std::wstring& process);

	void SetCallback(ProcessCreatedCallback);

	void RunMonitor();
	void CancelMonitor();

	bool IsRunning()const;

	std::exception_ptr GetLastException();

	void SetPollInterval(float interval);

	static int AdjustPrivilege();
};