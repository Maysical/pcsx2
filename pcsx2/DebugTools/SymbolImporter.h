// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#pragma once

#include "Config.h"
#include "SymbolGuardian.h"

#include <condition_variable>

class DebugInterface;

class SymbolImporter
{
public:
	SymbolImporter(SymbolGuardian& guardian);

	// These functions are used to receive events from the rest of the emulator
	// that are used to determine when symbol tables should be loaded, and
	// should be called from the CPU thread.
	void OnElfChanged(std::vector<u8> elf, const std::string& elf_file_name);
	void OnElfLoadedInMemory();
	void OnDebuggerOpened();
	void OnDebuggerClosed();

	// Delete all stored symbols and create some default built-ins. Should be
	// called from the CPU thread.
	void Reset();

	// Load the current ELF file and call AnalyseElf on it. Should be called
	// from the CPU thread.
	void LoadAndAnalyseElf(Pcsx2Config::DebugAnalysisOptions options);

	// Import symbols from the ELF file, nocash symbols, and scan for functions.
	// Should be called from the CPU thread.
	void AnalyseElf(
		std::vector<u8> elf,
		const std::string& elf_file_name,
		Pcsx2Config::DebugAnalysisOptions options,
		bool wait_until_elf_is_loaded);

	// Interrupt the import thread. Should be called from the CPU thread.
	void ShutdownWorkerThread();

	static void ClearExistingSymbols(ccc::SymbolDatabase& database, const Pcsx2Config::DebugAnalysisOptions& options);
	static bool ShouldClearSymbolsFromSourceByDefault(const std::string& source_name);

	static void ImportSymbols(
		ccc::SymbolDatabase& database,
		const ccc::ElfSymbolFile& elf,
		const std::string& nocash_path,
		const Pcsx2Config::DebugAnalysisOptions& options,
		const std::map<std::string, ccc::DataTypeHandle>& builtin_types,
		const std::atomic_bool* interrupt);

	static void ImportExtraSymbols(
		ccc::SymbolDatabase& database,
		const Pcsx2Config::DebugAnalysisOptions& options,
		const std::map<std::string, ccc::DataTypeHandle>& builtin_types,
		u32 importer_flags,
		const ccc::DemanglerFunctions& demangler,
		const std::atomic_bool* interrupt);

	static ccc::Result<bool> ImportNocashSymbols(
		ccc::SymbolDatabase& database,
		const std::string& file_path,
		u32 base_address,
		const std::map<std::string, ccc::DataTypeHandle>& builtin_types);

	static std::unique_ptr<ccc::ast::Node> GetBuiltInType(
		const std::string& name,
		ccc::ast::BuiltInClass bclass,
		const std::map<std::string, ccc::DataTypeHandle>& builtin_types);

	static void ScanForFunctions(
		ccc::SymbolDatabase& database, const ccc::ElfSymbolFile& elf, const Pcsx2Config::DebugAnalysisOptions& options);

protected:
	SymbolGuardian& m_guardian;

	bool m_symbol_table_loaded_on_boot = false;
	bool m_debugger_open = false;

	std::thread m_import_thread;
	std::atomic_bool m_interrupt_import_thread = false;

	std::mutex m_elf_loaded_in_memory_mutex;
	std::condition_variable m_elf_loaded_in_memory_condition_variable;
	bool m_elf_loaded_in_memory = false;

	std::map<std::string, ccc::DataTypeHandle> m_builtin_types;
};

extern SymbolImporter R5900SymbolImporter;
