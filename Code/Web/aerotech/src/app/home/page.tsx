export const dynamic = "force-dynamic";
export const revalidate = 0;

import { createServer } from "@/lib/supabase/server";
import LogoutButton from "@/components/LogoutButton";

export default async function HomePage() {
  const supabase = createServer();

  const {
    data: { user },
  } = await supabase.auth.getUser();

  if (!user) {
    return (
      <main className="min-h-screen flex items-center justify-center">
        <p className="text-lg text-gray-700">Not signed in.</p>
      </main>
    );
  }

  // Prefer display name from metadata; fallback to email
  const displayName =
    user.user_metadata?.full_name ||
    user.user_metadata?.name ||
    user.email ||
    "User";

  return (
    <main className="min-h-screen bg-brand-50">
      <div className="max-w-4xl mx-auto px-6 py-6 flex items-center justify-end">
        <LogoutButton />
      </div>

      <div className="min-h-[70vh] flex items-center justify-center px-4">
        <div className="text-center">
          <h1 className="text-4xl font-bold text-brand-700">
            Welcome to AeroTech {displayName}
          </h1>
          <p className="text-brand-800/70 mt-2">
            This will be your home page content later on.
          </p>
        </div>
      </div>
    </main>
  );
}
