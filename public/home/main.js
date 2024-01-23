const buttons = document.querySelectorAll(".expand");

buttons.forEach((button) => {
	button.addEventListener("click", () => {
		const scope = button.closest(".scope");

		scope.classList.toggle('active');
	})
})
