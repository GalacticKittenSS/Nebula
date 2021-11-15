#pragma once

#ifdef NB_WINDOWS

extern Nebula::Application* Nebula::createApplication();

int main(int argc, char** argv) {
	Nebula::Log::Init();

	NB_PROFILE_BEGIN_SESSION("Startup", "NebulaProfile-Startup.json");
	auto app = Nebula::createApplication();
	NB_PROFILE_END_SESSION();

	NB_PROFILE_BEGIN_SESSION("Runtime", "NebulaProfile-Runtime.json");
	app->run();
	NB_PROFILE_END_SESSION();

	NB_PROFILE_BEGIN_SESSION("Shutdown", "NebulaProfile-Shutdown.json");
	delete(app);
	NB_PROFILE_END_SESSION();
}

#endif // NB_WINDOWS