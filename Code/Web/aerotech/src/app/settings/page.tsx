import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import LogoutButton from "@/components/LogoutButton";

export default function SettingsPage() {
  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />

      <div className="px-4">
        <NavTabs />
      </div>

      <section className="max-w-3xl mx-auto px-4 py-10">
        <div className="flex items-center justify-between">
          <h2 className="text-3xl font-bold text-brand-700">Settings</h2>
          {/* Logout moved here */}
          <LogoutButton />
        </div>

        {/* space for future settings content */}
        <div className="mt-10 rounded-xl bg-white shadow-card p-6 border border-brand-100">
          <p className="text-brand-800/80">
            Settings page content will go here.
          </p>
        </div>
      </section>
    </main>
  );
}
